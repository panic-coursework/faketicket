<template>
  <Window title="查询车次" close @close="$router.push('/dashboard')" :status-bar="statusBar">
    <p><strong>{{ trainId }}</strong> {{ type }}</p>
    <div class="scroll">
      <div v-for="stop, i in stops" :key="i" class="ac">
        <p><strong>{{ stop }}</strong></p>
        <template v-if="i !== stops.length - 1">
          <p>{{ formatTime(edges[i].departure) }}</p>
          <p>票价: ¥{{ edges[i].price }} ↓ 余票: {{ edges[i].seatsRemaining }}</p>
          <p>{{ formatTime(edges[i].arrival) }}</p>
        </template>
      </div>
    </div>
    <p>
      <button @click="submit">删除车次</button>
      <button @click="release">开始售票</button>
    </p>
  </Window>
</template>

<script>
import { api, getUsername, statusBar } from '~/lib/fetch'

export default {
  inject: [ 'popup' ],
  data: () => ({
    trainId: '',
    type: '',
    edges: [],
    stops: [],
  }),
  async mounted () {
    if (!getUsername()) this.$router.push('/')

    const { date, trainId } = this.$router.currentRoute.params
    const res = await api(this, `train/${encodeURIComponent(date)}/${encodeURIComponent(trainId)}`)
    if (!res || res.error) return this.$router.push('/querytrain')
    for (const prop of [ 'trainId', 'type', 'edges', 'stops' ]) {
      this[prop] = res[prop]
    }
  },
  computed: {
    statusBar,
  },
  methods: {
    async submit () {
      const res = await api(this, 'remove-train', { trainId: this.trainId })
      if (res?.success) {
        this.popup('删除成功！')
        this.$router.push('/dashboard')
      }
    },
    async release () {
      const res = await api(this, 'release-train', { trainId: this.trainId })
      if (res?.success) {
        this.popup('发布成功！')
      }
    },
    formatTime (date) {
      date = new Date(date)
      const pad = num => String(num).padStart(2, '0')
      return `${pad(date.getMonth() + 1)}-${pad(date.getDate())},${pad(date.getHours())}:${pad(date.getMinutes())}`
    },
  },
}
</script>

<style scoped>
.fh {
  display: flex;
  justify-content: space-between;
}
.fh .field-row-stacked {
  width: 170px;
  margin-top: 0;
}

.scroll {
  max-height: 480px;
  overflow-y: auto;
}
</style>
