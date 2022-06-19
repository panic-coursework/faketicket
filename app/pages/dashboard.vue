<template>
  <Window title="主页" close @close="logout" :status-bar="statusBar">
    <fieldset>
      <legend>订　　票</legend>
      <button @click="$router.push('/travelplan')">旅行计划</button>
      <button @click="$router.push('/orderadd')">购买车票</button>
      <button @click="$router.push('/orders')">我的订单</button>
    </fieldset>
    <Spacer />
    <fieldset>
      <legend>列车管理</legend>
      <button @click="$router.push('/trainadd')">添加列车</button>
      <button @click="$router.push('/querytrain')">查询列车</button>
    </fieldset>
    <Spacer />
    <fieldset>
      <legend>用户操作</legend>
      <button @click="$router.push(`/usermod/${username}`)">我的信息</button>
      <button @click="$router.push('/userctl')">查找用户</button>
      <button @click="$router.push('/useradd')">添加用户</button>
    </fieldset>
  </Window>
</template>

<script>
import { getUsername, logout, statusBar } from '~/lib/fetch'

export default {
  inject: [ 'popup' ],
  data: () => ({
    username: getUsername(),
  }),
  methods: {
    logout,
  },
  computed: {
    statusBar,
  },
  mounted () {
    if (!getUsername()) this.$router.push('/')
  },
}
</script>

<style>

</style>
