<template>
  <Window title="购买车票" close @close="$router.push('/dashboard')" :status-bar="statusBar" style="width: 400px;">
    <form @submit.prevent="submit">
      <div class="field-row-stacked">
        <label for="trainId">车次</label>
        <input v-model="trainId" id="trainId" type="text" required>
      </div>
      <Spacer />
      <div class="fh">
        <div class="field-row-stacked">
          <label for="from">始发站</label>
          <input v-model="from" id="from" type="text" required>
        </div>
        <div class="field-row-stacked">
          <label for="to">终到站</label>
          <input v-model="to" id="to" type="text" required>
        </div>
      </div>
      <Spacer />
      <div class="fh">
        <div class="field-row-stacked">
          <label for="seats">数量</label>
          <input v-model="seats" id="seats" type="number" min="1" required>
        </div>
        <div class="field-row-stacked">
          <label for="date">出发日期</label>
          <input v-model="date" id="date" pattern="[0-9]{2}-[0-9]{2}" placeholder="07-01" type="text" required>
        </div>
      </div>
      <p class="fa">
        <span>
          <input type="checkbox" id="queue">
          <label for="queue">自动候补订票</label>
        </span>
        <button type="submit">订票</button>
      </p>
    </form>
  </Window>
</template>

<script>
import { api, getUsername, statusBar } from '~/lib/fetch'

export default {
  inject: [ 'popup' ],
  data: () => ({
    from: '',
    to: '',
    trainId: '',
    date: '',
    seats: '1',
    queue: false,
  }),
  computed: {
    statusBar,
  },
  mounted () {
    if (!getUsername()) this.$router.push('/')
  },
  methods: {
    async submit () {
      const { from, to, trainId, date, seats } = this
      const res = await api(this, 'orderadd', { from, to, trainId, date, seats })
      if (res?.success) {
        this.popup(res.status === 'success' ? `购票成功，已扣款 ${res.price} 元!` : `已加入候补队列！`)
        this.$router.push('/orders')
      }
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

.fa {
  display: flex;
  justify-content: space-between;
}
</style>
