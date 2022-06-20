const Koa = require('koa')
const Router = require('@koa/router')
const body = require('koa-body')
const static = require('koa-static')
const ticket = require('../..')

process.on('SIGINT', () => {
  // graceful shutdown in order to call C++ dtors
  process.exit(0)
})

const app = new Koa()
const router = new Router()
app.proxy = true
app.use(body())
app.use((ctx, next) => {
  if (ctx.method === 'POST') {
    if (ctx.headers['x-is-csrf'] !== 'false') {
      ctx.throw(400)
    }
  }
  return next()
})
app.use(router.routes()).use(router.allowedMethods())
app.use(static(__dirname + '/../dist/'))
app.use((ctx, next) => {
  ctx.path = '/'
  return next()
})
app.use(static(__dirname + '/../dist/'))
app.listen(process.env.PORT || 8080)

const requireLogin = async (ctx, next) => {
  const username = ctx.cookies.get('username')
  if (!username) {
    ctx.body = { error: 'not logged in' }
    return
  }
  ctx.username = username
  await next()
  if (ctx.body?.error?.includes('not logged in')) {
    ctx.cookies.set('username', 'x', { httpOnly: false, expires: new Date(0) })
  }
}

const usersLoggedIn = new Set()

router.post('/api/login', ctx => {
  const { username, password } = ctx.request.body
  if (!username || !password) ctx.throw(400)
  try {
    ctx.body = ticket.login({ username, password })
    usersLoggedIn.add(username)
    ctx.cookies.set('username', username, { httpOnly: false })
  } catch (e) {
    if (usersLoggedIn.has(username) && String(e).includes('already logged in')) {
      ctx.cookies.set('username', username, { httpOnly: false })
      ctx.body = { success: true }
      return
    }
    ctx.body = { error: String(e) }
  }
})

router.post('/api/logout', requireLogin, ctx => {
  try {
    ctx.cookies.set('username', 'x', { httpOnly: false, expires: new Date(0) })
    ctx.body = { success: true }
  } catch (e) {
    ctx.body = { error: String(e) }
  }
})

router.post('/api/orderadd', requireLogin, ctx => {
  const { from, to, trainId, date, seats } = ctx.request.body
  try {
    const res = ticket.buyTicket({
      from,
      to,
      train: trainId,
      date,
      seats: Number(seats),
      currentUser: ctx.username,
    })
    res.success = true
    ctx.body = res
  } catch (e) {
    ctx.body = { error: String(e) }
  }
})

router.get('/api/orders', requireLogin, ctx => {
  try {
    const result = ticket.queryOrder({ currentUser: ctx.username })
    ctx.body = { success: true, result }
  } catch (e) {
    ctx.body = { error: String(e) }
  }
})

router.post('/api/refund', requireLogin, ctx => {
  const { index } = ctx.request.body
  try {
    ctx.body = ticket.refundTicket({
      currentUser: ctx.username,
      index,
    })
  } catch (e) {
    ctx.body = { error: String(e) }
  }
})

router.post('/api/trainadd', requireLogin, ctx => {
  const { trainId, type, seats, begin, end, stations, edges } = ctx.request.body
  const parseTime = time => {
    const re = /^([0-9]{2}):([0-9]){2}(?:\+([0-3]))?$/
    const [ h, m, d ] = time.match(re)
    return [ h, m, d || 0 ]
  }
  const minFromHmd = ([ h, m, d ]) => {
    return h * 60 + m + d * 1440
  }
  for (const edge of edges) {
    for (const prop of [ 'departure', 'arrival' ]) {
      edge[prop] = parseTime(edge[prop])
      edge['min' + prop] = minFromHmd(edge[prop])
    }
  }
  const durations = edges.map(edge => edge.minarrival - edge.mindeparture)
  const stopoverTimes = edges.slice(1).map((_, i) => edges[i + 1].mindeparture - edges[i].minarrival)
  const pad = num => String(num).padStart(2, '0')
  try {
    ctx.body = ticket.addTrain({
      id: trainId,
      type,
      stops: stations.length,
      seats: Number(seats),
      dates: [ begin, end ],
      stations,
      durations,
      stopoverTimes,
      prices: edges.map(edge => edge.price).map(Number),
      departure: edges[0].departure.slice(0, 2).map(pad).join(':'),
    })
  } catch (e) {
    ctx.body = { error: String(e) }
  }
})

router.post('/api/travelplan', requireLogin, ctx => {
  const { from, to, date, type, sort } = ctx.request.body
  const call = type === 'direct' ? 'queryTicket' : 'queryTransfer'
  try {
    const result = ticket[call]({ from, to, date, sort })
    if (type === 'direct') {
      ctx.body = { success: true, result }
      return
    }
    if (!result.success) {
      ctx.body = { success: true, result: [] }
      return
    }
    ctx.body = {
      success: true,
      result: [ result.first, result.second ],
    }
  } catch (e) {
    ctx.body = { error: String(e) }
  }
})

router.post('/api/useradd', requireLogin, ctx => {
  const { username, password, name, email, privilege } = ctx.request.body
  try {
    ctx.body = ticket.addUser({ username, password, name, email, privilege: Number(privilege), currentUser: ctx.username })
  } catch (e) {
    ctx.body = { error: String(e) }
  }
})

router.get('/api/train/:date/:trainId', requireLogin, ctx => {
  const { date, trainId } = ctx.params
  try {
    ctx.body = ticket.queryTrain({ id: trainId, date })
  } catch (e) {
    ctx.body = { error: String(e) }
  }
})

router.post('/api/remove-train', requireLogin, ctx => {
  const { trainId } = ctx.request.body
  try {
    ctx.body = ticket.deleteTrain({ id: trainId })
  } catch (e) {
    ctx.body = { error: String(e) }
  }
})

router.post('/api/release-train', requireLogin, ctx => {
  const { trainId } = ctx.request.body
  try {
    ctx.body = ticket.releaseTrain({ id: trainId })
  } catch (e) {
    ctx.body = { error: String(e) }
  }
})

router.get('/api/user/:username', requireLogin, ctx => {
  const { username } = ctx.params
  try {
    ctx.body = ticket.queryProfile({ username, currentUser: ctx.username })
  } catch (e) {
    ctx.body = { error: String(e) }
  }
})

router.post('/api/usermod', requireLogin, ctx => {
  const { username, password, name, email, privilege } = ctx.request.body
  try {
    const obj = { username, password, name, email, privilege: Number(privilege), currentUser: ctx.username }
    if (!password) delete obj.password
    ticket.modifyProfile(obj)
    ctx.body = { success: true }
  } catch (e) {
    ctx.body = { error: String(e) }
  }
})
