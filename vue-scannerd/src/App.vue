<script>
export default {
  data() {
    return {
      apiKey: "7sOxe0FcXGgnL1UwDUjOmzGYSMSnYYncdQbsuHWe",
      FDC_SEARCH_API_ENDPOINT: "https://api.nal.usda.gov/fdc/v1/foods/search",

      searchOpen: false,
      searchLoading: false,
      searchValue: "",
      searchFocused: false,

      itemsPerPage: 10,
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
      querySelected: [],
    };
  },
  methods: {
    toggleSearch() {
      this.searchOpen = !this.searchOpen;

      if (this.searchOpen) {
        // set focus to search input field
        const el = this.$refs.searchInput.$el;
        const input = el.querySelector(
          "input:not([type=hidden]),textarea:not([type=hidden])"
        );
        if (input) {
          setTimeout(() => {
            input.focus();
          }, 0);
        }
      }
    },
    onSearchClicked() {
      this.loadItems({
        page: 1,
        itemsPerPage: this.itemsPerPage
      });
    },

    async loadItems({ page, itemsPerPage, sortBy }) {
      if (this.searchValue === null || this.searchValue == undefined) this.searchValue = "";

      if (this.searchValue.length == 0) {
        this.serverItems = [];
        this.totalItems = 0;
        this.searchLoading = false;
        return;
      }

      if (itemsPerPage === -1) {
        itemsPerPage = 100;
      }
      console.log(itemsPerPage);

      this.querySelected = [];

      var key = undefined;
      var order = undefined;

      if (sortBy !== undefined) {
        if (sortBy.length > 0) {
          key = sortBy[0]["key"];
          order = sortBy[0]["order"];
        }
      }

      var query = this.FDC_SEARCH_API_ENDPOINT + `?api_key=${this.apiKey}&query=${this.searchValue}`;

      if (key !== undefined && order !== undefined) {
        query += `&sortBy=${key}&sortOrder=${order}`;
      }

      query += `&pageNumber=${page}&pageSize=${itemsPerPage}`;

      this.searchLoading = true;
      await fetch(query).then(resp => resp.json()).then((json) => {
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
    hasSearchItems() {
      return this.searchOpen && this.serverItems.length != 0;
    }
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
        @click:append-inner="onSearchClicked" @keyup.enter="onSearchClicked" clearable ref="searchInput"></v-text-field>

      <v-btn :icon="searchOpen ? 'mdi-arrow-left' : 'mdi-magnify'" @click="toggleSearch"></v-btn>
    </v-toolbar>

    <div v-show="hasSearchItems" class="ml-4 mr-4 mb-4">
      <v-card>

        <v-data-table-server v-model="querySelected" v-model:items-per-page="itemsPerPage" :headers="headers"
          :item-value="item => `${item.fdcId}`" select-strategy="single" show-select return-object show-expand
          :items="serverItems" :items-length="totalItems" :loading="searchLoading" item-value="name"
          @update:options="loadItems">

          <template v-slot:expanded-row="{ columns, item }">
            <tr>
              <td :colspan="columns.length">
                {{ item.ingredients }}
              </td>
            </tr>
          </template>

        </v-data-table-server>



        <span>{{ querySelected }}</span>
      </v-card>
    </div>

  </v-app>
</template>

<style scoped></style>
