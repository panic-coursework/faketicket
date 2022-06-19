<template>
  <Window title="添加用户" close @close="$router.push('/dashboard')" :status-bar="statusBar">
    <form @submit.prevent="submit">
      <div class="field-row-stacked">
        <label for="username">用户名</label>
        <input v-model="username" id="username" type="text" required>
      </div>
      <div class="field-row-stacked">
        <label for="password">初始密码</label>
        <input v-model="password" id="password" type="password" required>
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
    username: '',
    password: '',
    name: '',
    email: '',
    privilege: '',
  }),
  computed: {
    statusBar,
  },
  mounted () {
    if (!getUsername()) this.$router.push('/')
  },
  methods: {
    async submit () {
      const { username, password, name, email, privilege } = this
      const res = await api(this, 'useradd', { username, password, name, email, privilege })
      if (res?.success) {
        this.popup('添加用户成功!')
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
