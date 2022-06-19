<template>
  <Window title="我的订单" close @close="$router.push('/dashboard')" :status-bar="statusBar">
    <div class="scroll">
      <p v-if="orders.length === 0">暂无订单</p>
      <fieldset v-for="order, i in orders" :key="i">
        <p>{{ formatTime(order.departure) }} {{ order.from }} → <strong>{{ order.trainId }}</strong> → {{ order.to }} {{ formatTime(order.arrival) }}</p>
        <p>单价: ¥{{ order.price }}, 数量: {{ order.seats }}, 总价: ¥{{ order.subTotal }}</p>
        <p>{{ statusString(order.status) }}</p>
        <p>
          <button @click="() => refund(i + 1)" :disabled="order.status === 'refunded'">退钱!</button>
        </p>
      </fieldset>
    </div>
  </Window>
</template>

<script>
import { api, getUsername, statusBar } from '~/lib/fetch'

export default {
  inject: [ 'popup' ],
  data: () => ({
    orders: [
      // {
      //   trainId: 'train',
      //   from: 'bj',
      //   to: 'sh',
      //   departure: new Date(),
      //   arrival: new Date(),
      //   price: 200,
      //   seats: 100,
      //   subTotal: 20000,
      //   status: 'success',
      // }
    ],
  }),
  computed: {
    statusBar,
  },
  mounted () {
    if (!getUsername()) return this.$router.push('/')
    this.loadData()
  },
  methods: {
    statusString (str) {
      switch (str) {
        case 'success': return '订票成功'
        case 'pending': return '候补中'
        case 'refunded': return '已退票'
        default: return '未知'
      }
    },
    formatTime (date) {
      date = new Date(date)
      const pad = num => String(num).padStart(2, '0')
      return `${pad(date.getMonth() + 1)}-${pad(date.getDate())},${pad(date.getHours())}:${pad(date.getMinutes())}`
    },
    async refund (index) {
      const res = await api(this, 'refund', { index })
      if (res?.success) {
        this.popup('钱款已退回原支付渠道')
        this.loadData()
      }
    },
    async loadData () {
      const res = await api(this, 'orders')
      if (res?.result) this.orders = res.result
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
</style>
