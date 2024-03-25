<script>
export default {
  data() {
    return {
      apiKey: "7sOxe0FcXGgnL1UwDUjOmzGYSMSnYYncdQbsuHWe",
      FDC_SEARCH_API_ENDPOINT: "https://api.nal.usda.gov/fdc/v1/foods/search",

      searchOpen: false,
      searchLoading: false,
      searchValue: "",

      itemsPerPage: 20,
      headers: [
        {
          title: 'Description',
          key: 'description',
          align: 'start',
          sortable: false,
        },
        { title: 'Category', key: 'foodCategory', align: 'end', sortable: false, },
        { title: 'Brand', key: 'brandOwner', align: 'end', sortable: false, },
        { title: 'FDC ID', key: 'fdcId', align: 'end', sortable: true, },
      ],
      serverItems: [],
      totalItems: 0,
    };
  },
  methods: {
    toggleSearch() {
      this.searchOpen = !this.searchOpen;
    },
    async onSearchClicked() {
      this.loadItems({
        page: 1,
        itemsPerPage: this.itemsPerPage,
        sortBy: undefined,
      });
    },

    async loadItems({ page, itemsPerPage, sortBy }) {
      if (this.searchValue.length == 0) return;

      var key = undefined;
      var order = undefined;

      if (sortBy !== undefined) {
        if (sortBy.length > 0) {
          key = sortBy[0]["key"];
          order = sortBy[0]["order"];
        }
      }

      var query = key === undefined || order === undefined ?
        this.FDC_SEARCH_API_ENDPOINT + `?api_key=${this.apiKey}&query=${this.searchValue}&pageNumber=${page}&pageSize=${itemsPerPage}` :
        this.FDC_SEARCH_API_ENDPOINT + `?api_key=${this.apiKey}&query=${this.searchValue}&pageNumber=${page}&pageSize=${itemsPerPage}&sortBy=${key}&sortOrder=${order}`;

      this.searchLoading = true;
      fetch(query).then(resp => resp.json()).then((json) => {
        this.serverItems = json["foods"];
        this.totalItems = json["totalHits"] >= 10000 ? 10000 : json["totalHits"];
        this.searchLoading = false;
        console.log(json);
      }).catch(e => {
        this.loading = false;
      });
    },
  },

  computed: {
    isSearchOpen() {
      return this.searchOpen;
    },
  },

  mounted() {

  },
};
// import HelloWorld from './components/HelloWorld.vue'
// import TheWelcome from './components/TheWelcome.vue'
</script>

<template>
  <v-app>
    <v-toolbar :collapse="!searchOpen">
      <v-btn icon="mdi-dots-vertical"></v-btn>

      <v-text-field v-model="searchValue" :disabled="searchLoading" v-show="isSearchOpen" :loading="searchLoading"
        append-inner-icon="mdi-magnify" density="compact" variant="solo" hide-details single-line
        @click:append-inner="onSearchClicked" @keyup.enter="onSearchClicked" clearable></v-text-field>

      <v-btn :icon="searchOpen ? 'mdi-arrow-left' : 'mdi-magnify'" @click="toggleSearch"></v-btn>
    </v-toolbar>

    <v-data-table-server v-show="isSearchOpen" v-model:items-per-page="itemsPerPage" :headers="headers"
      :items="serverItems" :items-length="totalItems" :loading="searchLoading" item-value="name"
      @update:options="loadItems"></v-data-table-server>

  </v-app>
</template>

<style scoped></style>
