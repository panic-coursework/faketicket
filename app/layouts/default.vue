<template>
  <div id="content">
    <Nuxt />
    <div id="popup" v-show="showPopup">
      <Window title="提示" close @close="showPopup = false">
        <p class="popup">{{ popupContent }}</p>
        <p class="ac">
          <button @click="showPopup = false">
            确认
          </button>
        </p>
      </Window>
    </div>
  </div>
</template>

<style scoped>
#content {
  font-family: "Pixelated MS Sans Serif", '宋体', 'SimSun' !important;
  margin-top: 128px;
  /* margin-bottom: 256px; */
  display: flex;
  justify-content: center;
  position: relative;
}
#popup {
  position: fixed;
  top: 200px;
  left: 46%;
  z-index: 99;
}
.popup {
  user-select: none;
}
</style>

<style>
.ar {
  text-align: right;
}
.ac {
  text-align: center;
}
.title-bar, .window, button, input, label, option, select, textarea, ul.tree-view {
  font-family: "Pixelated MS Sans Serif", '宋体', 'SimSun' !important;
}
</style>

<script>
let singleton
export default {
  data: () => ({
    showPopup: false,
    popupContent: '',
  }),
  provide: {
    popup (content) {
      singleton.showPopup = true
      singleton.popupContent = content
    },
  },
  mounted () {
    singleton = this
  },
}
</script>
