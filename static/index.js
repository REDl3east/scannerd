var tbl = document.getElementById("id-table-scans");

fetch("/last").then(async (resp) => {
  var json = await resp.json();
  var data = json["data"]
  console.log(data);

  for(var i = 0; i<data.length; i++){
    var tr = document.createElement("tr");
    var td = document.createElement("td");
    
    td.innerText = data[i];
    
    tr.append(td);
    tbl.append(tr);
  }

})


const ws = new WebSocket('ws://localhost:18080/ws')
ws.onopen = () => {
  ws.onmessage = (message) => {
    var tr = document.createElement("tr");
    var td = document.createElement("td");
    
    td.innerText = message.data;
    
    tr.prepend(td);
    tbl.prepend(tr);
  }  
}

