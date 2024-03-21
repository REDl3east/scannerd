var tbl = document.getElementById("id-table-scans");

fetch("/last").then(async (resp) => {
  var json = await resp.json();
  var data = json["data"]
  console.log(data);

  for (var i = 0; i < data.length; i++) {
    var tr = document.createElement("tr");
    var td1 = document.createElement("td");
    var td2 = document.createElement("td");

    td1.innerText = data[i];
    td2.innerText = "two";

    tr.append(td1);
    tr.append(td2);
    tbl.append(tr);
  }

})


// TODO: move to .env
const FDC_API_KEY = '7sOxe0FcXGgnL1UwDUjOmzGYSMSnYYncdQbsuHWe';
const FDC_SEARCH_API_ENDPOINT = 'https://api.nal.usda.gov/fdc/v1/foods/search/';
const FDC_FOOD_API_ENDPOINT = 'https://api.nal.usda.gov/fdc/v1/food/';

const ws = new WebSocket('ws://' + window.location.host + '/ws')
ws.onopen = () => {
  ws.onmessage = async (message) => {


    let query = await (await fetch(FDC_SEARCH_API_ENDPOINT + `?query=${message.data}` + "&api_key=" + FDC_API_KEY)).json();
    
    if(query["foods"].length > 0){
      let fdcID = query["foods"][0].fdcId;

      let food = await (await fetch(FDC_FOOD_API_ENDPOINT + `${fdcID}` + "?api_key=" + FDC_API_KEY)).json();

      console.log(food);
    }
    




    // .then(res1 => res1.json()).then((async json1 => {

    //   var fdcId = json1["foods"][0]["fdcId"];
    //   await fetch(FDC_FOOD_API_ENDPOINT + `${fdcId}` + "?api_key=" + FDC_API_KEY).then(res2 => res2.json()).then((json2 => {
    //     var tr = document.createElement("tr");
    //     var td1 = document.createElement("td");
    //     var td2 = document.createElement("td");
  
    //     td1.innerText = message.data;
    //     td2.innerText = fdcId;
  
    //     tr.append(td1);
    //     tr.append(td2);
    //     tbl.prepend(tr);
  
  
    //     console.log(json2);
    //   }));



    // }))


  }
}

