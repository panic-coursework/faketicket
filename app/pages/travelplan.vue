<template>
  <Window title="旅行计划" close @close="$router.push('/dashboard')" :status-bar="statusBar" style="width: 400px;">
    <form @submit.prevent="submit">
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
      <div class="field-row-stacked">
        <label for="date">出发日期</label>
        <input v-model="date" id="date" pattern="[0-9]{2}-[0-9]{2}" placeholder="07-01" type="text" required>
      </div>
      <p class="fa">
        <span>
          <input id="direct" type="radio" name="type" v-model="type" value="direct">
          <label for="direct">直达</label>
          <span>&nbsp;</span>
          <input id="transfer" type="radio" name="type" v-model="type" value="transfer">
          <label for="transfer">中转</label>
          <span>&nbsp;</span>
          <input id="time" type="radio" name="sort" v-model="sort" value="time">
          <label for="time">时间优先</label>
          <span>&nbsp;</span>
          <input id="cost" type="radio" name="sort" v-model="sort" value="cost">
          <label for="cost">费用优先</label>
        </span>
        <button type="submit">查询</button>
      </p>
    </form>
    <Window class="result" title="查询结果" close @close="showResult = false" v-show="showResult">
      <p v-if="!result || result.length === 0"><strong>没有生成好的经由！</strong></p>
      <div class="scroll" v-else>
        <fieldset v-for="range, i in result" :key="i">
          <p>{{ formatTime(range.timeDeparture) }} {{ range.from }} → <strong>{{ range.trainId }}</strong> → {{ range.to }} {{ formatTime(range.timeArrival) }}</p>
          <p>单价: ¥{{ range.price }}, 余票: {{ range.ticketsAvailable }}</p>
          <p>
            <button @click="$router.push('/orderadd')" :disabled="range.ticketsAvailable === 0">买票!</button>
          </p>
        </fieldset>
      </div>
    </Window>
  </Window>
</template>

<script>
import { api, getUsername, statusBar } from '~/lib/fetch'

export default {
  inject: [ 'popup' ],
  data: () => ({
    from: '',
    to: '',
    date: '',
    type: 'direct',
    sort: 'time',
    showResult: false,
    result: [],
  }),
  computed: {
    statusBar,
  },
  mounted () {
    if (!getUsername()) this.$router.push('/')
  },
  methods: {
    async submit () {
      const { from, to, date, type, sort } = this
      const res = await api(this, 'travelplan', { from, to, date, type, sort })
      if (res?.success) {
        this.popup('查询成功！')
        this.result = res.result
        this.showResult = true
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

.fa {
  display: flex;
  justify-content: space-between;
}

.scroll {
  max-height: 480px;
  overflow-y: auto;
}
.result {
  position: fixed;
  top: 200px;
}
</style>
