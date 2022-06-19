<template>
  <Window title="查询列车" close @close="$router.push('/dashboard')" :status-bar="statusBar">
    <form @submit.prevent="submit">
      <div class="field-row-stacked">
        <label for="trainId">车次</label>
        <input v-model="trainId" id="trainId" type="text" required>
      </div>
      <div class="field-row-stacked">
        <label for="date">发车日期</label>
        <input v-model="date" id="date" pattern="[0-9]{2}-[0-9]{2}" placeholder="07-01" type="text" required>
      </div>
      <p class="ar">
        <button type="submit" class="fr">查询</button>
      </p>
    </form>
  </Window>
</template>

<script>
import { getUsername, statusBar } from '~/lib/fetch'

export default {
  inject: [ 'popup' ],
  data: () => ({
    trainId: '',
    date: '',
  }),
  computed: {
    statusBar,
  },
  mounted () {
    if (!getUsername()) this.$router.push('/')
  },
  methods: {
    submit () {
      this.$router.push(`/train/${encodeURIComponent(this.date)}/${encodeURIComponent(this.trainId)}`)
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
</style>
