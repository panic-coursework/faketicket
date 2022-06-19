<template>
  <Window title="添加列车" close @close="$router.push('/dashboard')" :status-bar="statusBar" style="width: 400px;">
    <form @submit.prevent="submit">
      <div class="scroll">
        <fieldset>
          <legend>基本信息</legend>
          <div class="field-row-stacked">
            <label for="trainId">车次</label>
            <input v-model="trainId" id="trainId" type="text" required>
          </div>
          <Spacer />
          <div class="fh">
            <div class="field-row-stacked">
              <label for="type">类型</label>
              <input v-model="type" id="type" type="text" required>
            </div>
            <div class="field-row-stacked">
              <label for="seats">座位数</label>
              <input v-model="seats" id="seats" type="number" required>
            </div>
          </div>
          <Spacer />
          <div class="fh">
            <div class="field-row-stacked">
              <label for="begin">开行起始日期</label>
              <input v-model="begin" id="begin" pattern="[0-9]{2}-[0-9]{2}" placeholder="07-01" type="text" required>
            </div>
            <div class="field-row-stacked">
              <label for="end">开行结束日期</label>
              <input v-model="end" id="end" pattern="[0-9]{2}-[0-9]{2}" placeholder="07-01" type="text" required>
            </div>
          </div>
        </fieldset>
        <Spacer />
        <fieldset>
          <legend>过　　站</legend>
          <div v-for="_, i in stations" :key="i">
            <div class="field-row-stacked">
              <label :for="`station-${i}`">站名</label>
              <input v-model="stations[i]" :id="`station-${i}`" type="text" required>
            </div>
            <template v-if="i !== stations.length - 1">
              <Spacer style="margin: 32px 0;" />
              <div class="fh">
                <div class="field-row-stacked">
                  <label :for="`edge-${i}-departure`">发车时间</label>
                  <input v-model="edges[i].departure" :id="`edge-${i}-departure`" type="text" pattern="[0-9]{2}:[0-9]{2}(\+[0-3])?" placeholder="17:35+1" required>
                </div>
                <div class="field-row-stacked">
                  <label :for="`edge-${i}-arrival`">到站时间</label>
                  <input v-model="edges[i].arrival" :id="`edge-${i}-arrival`" type="text" pattern="[0-9]{2}:[0-9]{2}(\+[0-3])?" placeholder="17:35+1" required>
                </div>
              </div>
              <Spacer />
              <div class="field-row-stacked">
                <label :for="`edge-${i}-price`">价格（元）</label>
                <input v-model="edges[i].price" :id="`edge-${i}-price`" type="number" min="0" step="1" required>
              </div>
              <Spacer style="margin: 32px 0;" />
            </template>
          </div>
          <p>
            <button @click.prevent="addEdge">添加车站</button>
            <button @click.prevent="removeEdge" :disabled="edges.length === 1">删除车站</button>
          </p>
        </fieldset>
      </div>
      <p class="ar">
        <button type="submit" class="fr">添加</button>
      </p>
    </form>
  </Window>
</template>

<script>
import { api, getUsername, statusBar } from '~/lib/fetch'

export default {
  inject: [ 'popup' ],
  data: () => ({
    trainId: '',
    type: '',
    seats: '',
    begin: '',
    end: '',
    stations: [ '', '' ],
    edges: [ {
      departure: '',
      arrival: '',
      price: '',
    } ],
  }),
  computed: {
    statusBar,
  },
  mounted () {
    if (!getUsername()) this.$router.push('/')
  },
  methods: {
    async submit () {
      const { trainId, type, seats, begin, end, stations, edges } = this
      const res = await api(this, 'trainadd', { trainId, type, seats, begin, end, stations, edges })
      if (res?.success) {
        this.popup('添加成功!')
        this.$router.push(`/train/${encodeURIComponent(begin)}/${encodeURIComponent(trainId)}`)
      }
    },
    addEdge () {
      this.stations.push('')
      this.edges.push({
        departure: '',
        arrival: '',
        price: '',
      })
    },
    removeEdge () {
      this.stations.pop()
      this.edges.pop()
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
  width: 160px;
  margin-top: 0;
}
.scroll {
  max-height: 512px;
  overflow-y: scroll;
}
</style>
