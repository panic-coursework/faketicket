<template>
  <Window title="登录" style="width: 320px;">
    <form @submit.prevent="login">
      <div class="field-row-stacked">
        <label for="username">用户名</label>
        <input v-model="username" id="username" type="text" ref="username" required>
      </div>
      <div class="field-row-stacked">
        <label for="password">密码</label>
        <input v-model="password" id="password" type="password" required>
      </div>
      <p class="ar">
        <button type="submit" class="fr">登录</button>
      </p>
    </form>
  </Window>
</template>

<style scoped>
.field-row-stacked {
  width: 100%;
}
</style>

<script>
import { api, getUsername } from '~/lib/fetch'

export default {
  name: 'IndexPage',
  data: () => ({
    username: '',
    password: '',
  }),
  inject: [ 'popup' ],
  methods: {
    async login () {
      const res = await api(this, 'login', {
        username: this.username,
        password: this.password,
      })
      if (res?.success) {
        this.popup('登录成功!')
        this.$router.push('/dashboard')
      }
    },
  },
  mounted () {
    if (getUsername()) this.$router.push('/dashboard')
    this.$refs.username.focus()
  },
}
</script>
