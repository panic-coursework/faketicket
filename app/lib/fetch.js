import cookie from 'cookie'

export async function api (self, url, args) {
  try {
    const init = {
      credentials: 'include',
      method: args ? 'post' : 'get',
    }
    if (args) {
      init.body = JSON.stringify(args)
      init.headers = {
        'Content-Type': 'application/json',
        'X-Is-Csrf': 'false',
      }
    }
    const res = await (await fetch('/api/' + url, init)).json()
    if (res.error) self.popup(res.error)
    return res
  } catch (e) {
    self.popup(String(e))
  }
}

export function getUsername () {
  return cookie.parse(document.cookie).username
}

export async function logout () {
  await api(this, 'logout', {})
  this.$router.push('/')
}

export function statusBar () {
  return [ '当前用户: ' + getUsername(), '仅限管内车', '交局闵段' ]
}
