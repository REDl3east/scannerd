<script>
export default {
  data() {
    return {
      apiKey: "7sOxe0FcXGgnL1UwDUjOmzGYSMSnYYncdQbsuHWe",
      FDC_SEARCH_API_ENDPOINT: "https://api.nal.usda.gov/fdc/v1/foods/search",
      FDC_FOOD_API_ENDPOINT: "https://api.nal.usda.gov/fdc/v1/food/",

      searchOpen: true,
      searchLoading: false,
      searchValue: "",
      searchFocused: false,

      itemsPerPage: 10,
      headers: [
        {
          title: "Description",
          key: "description",
          align: "start",
          sortable: false,
        },
        { title: "Category", key: "foodCategory", align: "end", sortable: false },
        { title: "Brand", key: "brandOwner", align: "end", sortable: false },
        { title: "FDC ID", key: "fdcId", align: "end", sortable: true },
      ],
      serverItems: [],
      totalItems: 0,
      querySelected: [],
      websocket_url: "ws://" + location.host + "/ws",
      websocket: null,
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
        itemsPerPage: this.itemsPerPage,
      });
    },

    async loadItems({ page, itemsPerPage, sortBy }) {
      if (this.searchValue === null || this.searchValue == undefined)
        this.searchValue = "";

      if (this.searchValue.length == 0) {
        this.serverItems = [];
        this.totalItems = 0;
        this.searchLoading = false;
        return;
      }

      if (itemsPerPage === -1) {
        itemsPerPage = 25;
      }

      this.querySelected = [];

      var key = undefined;
      var order = undefined;

      if (sortBy !== undefined) {
        if (sortBy.length > 0) {
          key = sortBy[0]["key"];
          order = sortBy[0]["order"];
        }
      }

      var query =
        this.FDC_SEARCH_API_ENDPOINT +
        `?api_key=${this.apiKey}&query=${this.searchValue}`;

      if (key !== undefined && order !== undefined) {
        query += `&sortBy=${key}&sortOrder=${order}`;
      }

      query += `&pageNumber=${page}&pageSize=${itemsPerPage}`;

      this.searchLoading = true;
      await fetch(query)
        .then((resp) => resp.json())
        .then((json) => {
          this.serverItems = json["foods"];
          this.totalItems = json["totalHits"] >= 10000 ? 10000 : json["totalHits"];
          this.searchLoading = false;
          console.log(json);
        })
        .catch((e) => {
          this.searchLoading = false;
        });
    },
    cleanup(items) {
      console.log(items);
      let newItems = items.flatMap((item) => {
        let newItem = Object.assign({}, item);

        if(newItem.unitName == "G"){
          newItem.unitName = "g";
          newItem.nutrientNumber = newItem.nutrientNumber / 100;
        }
        if(newItem.unitName == "KCAL"){
          newItem.unitName = "";

        }

        if (newItem.nutrientName == "Protein") {
          return newItem;
        }
        if (newItem.nutrientName == "Energy") {
          newItem.nutrientName = "Calories"
          return newItem;
        }



        return undefined;
      });

      newItems = newItems.filter(function (element) {
        return element !== undefined;
      });

      return newItems;
    },
  },

  computed: {
    isSearchOpen() {
      return this.searchOpen;
    },
    hasSearchItems() {
      return this.searchOpen && this.serverItems.length != 0;
    },
    hasSelected() {
      return !this.searchLoading && this.querySelected.length > 0;
    },
  },

  watch: {
    async querySelected(query) {
      if (query.length <= 0) return;
      let value = query[0];

      var query = this.FDC_FOOD_API_ENDPOINT + value.fdcId + `?api_key=${this.apiKey}`;

      this.searchLoading = true;
      await fetch(query)
        .then((resp) => resp.json())
        .then((json) => {
          this.searchLoading = false;
          console.log(json);
        })
        .catch((e) => {
          this.searchLoading = false;
        });
    },
  },

  mounted() {
    this.socket = new WebSocket(this.websocket_url);
    this.socket.onopen = (event) => {
      this.socket.onmessage = (event) => {
        this.searchOpen = true;
        this.searchValue = event.data;
        this.onSearchClicked();
      };
    };
  },
};
</script>

<template>
  <v-app>
    <v-toolbar :collapse="!searchOpen">
      <v-btn icon="mdi-dots-vertical"></v-btn>

      <v-text-field
        v-model="searchValue"
        :disabled="searchLoading"
        v-show="isSearchOpen"
        :loading="searchLoading"
        append-inner-icon="mdi-magnify"
        density="compact"
        variant="solo"
        hide-details
        single-line
        @click:append-inner="onSearchClicked"
        @keyup.enter="onSearchClicked"
        clearable
        ref="searchInput"
      ></v-text-field>

      <v-btn
        :icon="searchOpen ? 'mdi-arrow-left' : 'mdi-magnify'"
        @click="toggleSearch"
      ></v-btn>
    </v-toolbar>

    <div v-show="hasSearchItems" class="ml-4 mr-4 mb-4">
      <v-card>
        <v-data-table-server
          v-model="querySelected"
          v-model:items-per-page="itemsPerPage"
          :headers="headers"
          :item-value="(item) => `${item.fdcId}`"
          select-strategy="single"
          show-select
          return-object
          show-expand
          :items="serverItems"
          :items-length="totalItems"
          :loading="searchLoading"
          @update:options="loadItems"
        >
          <template v-slot:expanded-row="{ columns, item }">
            <tr>
              <td :colspan="columns.length">
                <v-table>
                  <tbody>
                    <tr v-for="i in cleanup(item.foodNutrients)">
                      <td>{{ i.nutrientName }}</td>
                      <td>{{ i.nutrientNumber + " " + i.unitName }}</td>
                    </tr>
                  </tbody>
                </v-table>
              </td>
            </tr>
            <!-- <tr>
              <td :colspan="columns.length">
                {{ Object.hasOwn(item, 'foodNutrients') ? item.foodNutrients : "N/A" }}
              </td>
            </tr> -->
          </template>
        </v-data-table-server>
      </v-card>

      <v-card v-show="hasSelected" class="mt-4"> </v-card>
    </div>
  </v-app>
</template>

<style scoped></style>
