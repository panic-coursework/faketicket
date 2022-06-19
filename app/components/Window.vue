<template>
  <div class="window">
    <div class="title-bar">
      <div class="title-bar-text">{{ title }} - 交局闵段票务系统</div>
      <div class="title-bar-controls">
        <button aria-label="Close" v-if="close" @click="$emit('close')"></button>
      </div>
    </div>
    <div class="window-body">
      <slot />
    </div>
    <div class="status-bar" v-if="statusBar.length > 0">
      <p v-for="text, i in statusBar" :key="i" class="status-bar-field">
        {{ text }}
      </p>
    </div>
  </div>
</template>

<style scoped>
.title-bar-text {
  user-select: none;
}
</style>

<script>
export default {
  props: {
    title: String,
    statusBar: {
      type: Array,
      default: () => [],
    },
    close: {
      type: Boolean,
      default: false,
    },
  },
}
</script>
