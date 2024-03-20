var tbl = document.getElementById("id-table-scans");

fetch("/first/10").then(async (resp) => {
  var json = await resp.json();
  var data = json["data"]
  console.log(data);

  for(var i = 0; i<data.length; i++){
    var tr = document.createElement("tr");
    var td = document.createElement("td");
    
    td.innerText = data[i];
    
    tr.appendChild(td);
    tbl.appendChild(tr);
  }

})


