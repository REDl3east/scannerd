const socket = new WebSocket("ws://" + location.host + "/ws");

socket.addEventListener('open', (event) => {
    console.log('Connected to WebSocket server');
});

socket.addEventListener('message', (event) => {
    let barcode = event.data;
    console.log('Message from server: ', barcode);
    set_to_loading(barcode);
});

socket.addEventListener('close', (event) => {
    console.log('Disconnected from WebSocket server');
});

socket.addEventListener('error', (event) => {
    console.error('WebSocket error observed:', event);
});

const API_KEY = "7sOxe0FcXGgnL1UwDUjOmzGYSMSnYYncdQbsuHWe";
const FDC_SEARCH_API_ENDPOINT = "https://api.nal.usda.gov/fdc/v1/foods/search";
const FDC_FOOD_API_ENDPOINT = "https://api.nal.usda.gov/fdc/v1/food/";


async function set_to_loading(barcode) {
    document.getElementById('app-loading').removeAttribute('hidden');
    document.getElementById('app-start').setAttribute('hidden', '');
    document.getElementById('app-loading-fail').setAttribute('hidden', '');
    document.getElementById('app-loading-success').setAttribute('hidden', '');
    document.getElementById('app-no-results').setAttribute('hidden', '');

    var search_query = FDC_SEARCH_API_ENDPOINT + `?api_key=${API_KEY}&query=${barcode}`;
    await fetch(search_query)
        .then((resp) => resp.json())
        .then(async (json) => {
            if (json.totalHits == 0 || json["foods"].length == 0) {
                set_to_no_results();
                return;
            }
            const fdcid = json["foods"][0]["fdcId"]
            var food_query = FDC_FOOD_API_ENDPOINT + `${fdcid}?api_key=${API_KEY}`;
            await fetch(food_query)
                .then((resp) => resp.json())
                .then((json) => {
                    set_loading_success(json)
                })
                .catch((e) => {
                    set_loading_fail();
                });

        })
        .catch((e) => {
            set_loading_fail();
        });

}

function set_loading_success(result) {
    const nutrients = result["labelNutrients"];
    console.log(result["labelNutrients"]);


    document.getElementById('app-loading-success').removeAttribute('hidden');
    document.getElementById('app-start').setAttribute('hidden', '');
    document.getElementById('app-loading-fail').setAttribute('hidden', '');
    document.getElementById('app-no-results').setAttribute('hidden', '');
    document.getElementById('app-loading').setAttribute('hidden', '');



}

function set_to_no_results() {
    document.getElementById('app-no-results').removeAttribute('hidden');
    document.getElementById('app-start').setAttribute('hidden', '');
    document.getElementById('app-loading-fail').setAttribute('hidden', '');
    document.getElementById('app-loading-success').setAttribute('hidden', '');
    document.getElementById('app-loading').setAttribute('hidden', '');
}

function set_loading_fail() {
    document.getElementById('app-loading-fail').removeAttribute('hidden');
    document.getElementById('app-start').setAttribute('hidden', '');
    document.getElementById('app-no-results').setAttribute('hidden', '');
    document.getElementById('app-loading-success').setAttribute('hidden', '');
    document.getElementById('app-loading').setAttribute('hidden', '');
}