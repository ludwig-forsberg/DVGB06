const urlParams = new URLSearchParams(window.location.search);
var tourId = urlParams.get("id");
if(tourId == null || tourId == ""){
    window.location = ".";
}else{
    function addRowToTable(table, data) {
        let row = table.insertRow(table.rows.length);
    
        for (let i = 0; i < data.length; i++) {
            let cell = row.insertCell(i);
            if(typeof(data[i]) == "object"){
                cell.appendChild(data[i]);
            }else{
    
                cell.innerHTML = data[i];
            }
        }
    
        for(let i = data.length; i < table.rows[0].cells.length; i++){
            row.insertCell(i);
        }
    
        return row;
    }

    users = {};

    function updateTour(tour_data){
        //TODO: Finish

        if(tour_data["matches"] != undefined){
            for(let i = 0; i < tour_data["matches"].length; i++){
                let match = tour_data["matches"][i];

                let matchId = match["id"];

                let team1 = "";
                let team2 = "";
                let sets = ["", "", "", "", ""];

                if(match["sets"] != undefined){
                    for(let u = 0; u < match["sets"].length; u++){
                        let set = match["sets"][u];
                        sets[u] = `${set["team-1-points"]} - ${set["team-2-points"]}`;
                    }
                }

                let matchNavigator = document.createElement("a");
                matchNavigator.href = "/ingame.html?id="+matchId;
                matchNavigator.innerHTML = "link";

                let qrDiv = document.createElement("div");
                let qrToggleButton = document.createElement("button-component");
                qrToggleButton.innerHTML = "Show";
                let qrCodeContainer = document.createElementNS("http://www.w3.org/2000/svg", "svg");
                new QRCode(qrCodeContainer, matchNavigator.href);
                qrDiv.appendChild(qrToggleButton);
                qrDiv.appendChild(qrCodeContainer);

                qrCodeContainer.style.display = "none";

                qrToggleButton.onclick = ()=>{
                    if(qrCodeContainer.style.display == "none"){
                        qrCodeContainer.style.display = "";
                        qrToggleButton.setAttribute("text", "Hide");
                    }else{
                        qrCodeContainer.style.display = "none";
                        qrToggleButton.setAttribute("text", "Show");
                    }
                }

                let standing = `${match["team-1-set-points"]} - ${match["team-2-set-points"]}`;

                addRowToTable(matchTable, [team1, team2, sets[0], sets[1], sets[2], sets[3], sets[4], standing, matchNavigator, qrDiv]);
            }
        }
    }

    function addUser(userData) {
        let temp = users[userData["id"]];
        let teamId = userData["team-id"];
        let user = (temp == undefined ? function () {
            let newUser = {};
            newUser.id = userData["id"];
            users[newUser.id] = newUser;
            return newUser;
        }() : temp);
        user.name = userData["name"];
        user.teamId = userData["team-id"];
    }

    window.addEventListener('load', (event) => {
        var wsproto = (location.protocol === 'https:') ? 'wss:' : 'ws:';
        connection = new WebSocket(wsproto + '//' + window.location.host + '/websocket');

        let userId = sessionStorage.getItem('id');
        let userName = sessionStorage.getItem('name');
        let authenticated = userId != null;
        if(authenticated){
            userId = parseInt(userId);
            header.innerHTML = `Tour ${tourId} - ${userName}`;
        }else{
            header.innerHTML = `Tour ${tourId}`;
        }

        connection.onmessage = function (e) {
            if (e.data != "") {
                let json = JSON.parse(e.data);
                console.log(json);
                for(const [method, method_data] of Object.entries(json)){
                    switch(method){
                        case "auth":
                            sessionStorage.setItem('id', method_data["id"]);
                            sessionStorage.setItem('name', method_data["name"]);
                            header.innerHTML =  `Tour ${tourId} - ${method_data["name"]}`;
                            connection.send(JSON.stringify({
                                "get": {
                                    "users":[],
                                    "teams":[],
                                    "tours":[{
                                        "id": tourId,
                                        "matches":[]
                                    }]
                                }
                            }));
                            break;
                        case "get":
                            for(const [data_type, data] of Object.entries(method_data)){
                                switch(data_type){
                                    case "users":
                                        for(let i = 0; i < data.length; i++){
                                        }
                                        break;
                                    case "teams":
                                        for(let i = 0; i < data.length; i++){
                                        }
                                        break;
                                    case "tours":
                                        for(let i = 0; i < data.length; i++){
                                            updateTour(data[i]);
                                        }
                                        break;
                                }
                            }
                            break;
                        case "update":
                            for(const [data_type, data] of Object.entries(method_data)){
                                switch(data_type){
                                    case "users":
                                        for(let i = 0; i < data.length; i++){
                                        }
                                        break;
                                    case "teams":
                                        for(let i = 0; i < data.length; i++){
                                        }
                                        break;
                                    case "tours":
                                        for(let i = 0; i < data.length; i++){
                                        }
                                        break;
                                }
                            }
                            break;
                        case "create":
                            for(const [data_type, data] of Object.entries(method_data)){
                                switch(data_type){
                                    case "teams":
                                        for(let i = 0; i < data.length; i++){
                                        }
                                        break;
                                    case "tours":
                                        for(let i = 0; i < data.length; i++){
                                        }
                                        break;
                                }
                            }
                            break;
                        case "delete":
                            for(const [data_type, data] of Object.entries(method_data)){
                                switch(data_type){
                                    case "teams":
                                        for(let i = 0; i < data.length; i++){
                                        }
                                        break;
                                    case "tours":
                                        for(let i = 0; i < data.length; i++){
                                        }
                                        break;
                                }
                            }
                            break;
                    }
                }
            }
        }
        connection.onerror = function (error) {
            console.error(error);
            connection.close();
        }

        connection.onopen = function (e) {
            if (authenticated) {
                connection.send(JSON.stringify({
                    "auth": {
                        "id": userId,
                        "name": userName
                    }
                }));
            }
            
        }
    });
}