<template>
  <Window title="修改用户" close @close="$router.push('/dashboard')" :status-bar="statusBar">
    <form @submit.prevent="submit">
      <div class="field-row-stacked">
        <label for="username">用户名</label>
        <input v-model="username" id="username" type="text" disabled>
      </div>
      <div class="field-row-stacked">
        <label for="password">重置密码（不变可留空）</label>
        <input v-model="password" id="password" type="password">
      </div>
      <div class="field-row-stacked">
        <label for="name">姓名</label>
        <input v-model="name" id="name" type="text" required>
      </div>
      <div class="field-row-stacked">
        <label for="email">电子邮件地址</label>
        <input v-model="email" id="email" type="email" required>
      </div>
      <div class="field-row-stacked">
        <label for="privilege">权限级别</label>
        <input v-model="privilege" id="privilege" type="number" min="0" required>
      </div>
      <p class="ar">
        <button type="submit" class="fr">修改</button>
      </p>
    </form>
  </Window>
</template>

<script>
import { api, getUsername, statusBar } from '~/lib/fetch'

export default {
  inject: [ 'popup' ],
  data: () => ({
    username: '',
    password: '',
    name: '',
    email: '',
    privilege: '',
  }),
  async mounted () {
    if (!getUsername()) this.$router.push('/')

    const { username } = this.$router.currentRoute.params
    this.username = username
    const res = await api(this, `user/${encodeURIComponent(username)}`)
    if (!res || res.error) return this.$router.push('/userctl')
    for (const prop of [ 'name', 'email', 'privilege' ]) {
      this[prop] = res[prop]
    }
  },
  computed: {
    statusBar,
  },
  methods: {
    async submit () {
      const { username, password, name, email, privilege } = this
      const res = await api(this, 'usermod', { username, password, name, email, privilege })
      if (res?.success) {
        this.popup('更新用户成功！')
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
</style>
