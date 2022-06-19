<template>
  <Window title="查找用户" close @close="$router.push('/dashboard')" :status-bar="statusBar">
    <form @submit.prevent="submit">
      <div class="field-row-stacked">
        <label for="username">用户名</label>
        <input v-model="username" id="username" type="text" required>
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
    username: '',
  }),
  computed: {
    statusBar,
  },
  mounted () {
    if (!getUsername()) this.$router.push('/')
  },
  methods: {
    async submit () {
      this.$router.push(`/usermod/${encodeURIComponent(this.username)}`)
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
