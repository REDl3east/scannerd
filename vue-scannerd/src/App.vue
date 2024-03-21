<script>
export default {

  data() {
    return {
      count: 0,
    };
  },
  methods: {
    increment() {
      this.count++;
    },
  },

  computed: {
    isOdd() {
      return this.count % 2 == 0 ? "Even" : "Odd&nbsp;&nbsp;";
    },
  },

  mounted() {
    console.log(`The initial count is ${this.count}.`);
  },
};
// import HelloWorld from './components/HelloWorld.vue'
// import TheWelcome from './components/TheWelcome.vue'
</script>

<template>
  <v-app>
    <v-app-bar :elevation="12">
      <template v-slot:prepend>
        <v-app-bar-nav-icon></v-app-bar-nav-icon>
      </template>

      <v-app-bar-title>Application Bar</v-app-bar-title>

      <template v-slot:append>
        <v-btn icon="mdi-account-key"></v-btn>

        <v-btn icon="mdi-magnify"></v-btn>

        <v-btn icon="mdi-dots-vertical"></v-btn>
      </template>
    </v-app-bar>
  </v-app>
</template>

<style scoped>
h1 {
  color: red;
}
</style>
