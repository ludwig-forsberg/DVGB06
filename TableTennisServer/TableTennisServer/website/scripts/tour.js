const urlParams = new URLSearchParams(window.location.search);
tour = {};

tour.id = urlParams.get("id");
let isOwner = false;
if(tour.id == null || tour.id == ""){
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
    teams = {};
    matches = {};

    function getPlayerText(matchId, team1){
        if(team1){
            switch(matchId){
                case 0:
                    return "A1";
                case 1:
                    return "A3";
                case 2:
                    return "A2";
                case 3:
                    return "A4";
                case 4:
                    return "A1";
                case 5:
                    return "A2";
                case 6:
                    return "A3";
                case 7:
                    return "A4";
                case 8:
                    return "D1";
                case 9:
                    return "D2";
                case 10:
                    return "A1/A5";
                case 11:
                    return "A2/A5";
                case 12:
                    return "A3/A5";
                case 13:
                    return "A4/A5";
            }
        }else{
            switch(matchId){
                case 0:
                    return "B3";
                case 1:
                    return "B1";
                case 2:
                    return "B4";
                case 3:
                    return "B2";
                case 4:
                    return "B2";
                case 5:
                    return "B1";
                case 6:
                    return "B4";
                case 7:
                    return "B3";
                case 8:
                    return "D1";
                case 9:
                    return "D2";
                case 10:
                    return "B1/B5";
                case 11:
                    return "B2/B5";
                case 12:
                    return "B3/B5";
                case 13:
                    return "B4/B5";
            }
        }
        return "";
    }

    function updateUser(user_data){
        let temp = users[user_data["id"]];
        let teamId = user_data["team-id"];
        let user = (temp == undefined ? function () {
            let newUser = {};
            newUser.id = user_data["id"];
            users[newUser.id] = newUser;
            return newUser;
        }() : temp);
        user.name = user_data["name"];
        user.teamId = user_data["team-id"];
    }

    function updateTeam(team_data){
        let temp = teams[team_data["id"]];
        let team = (temp == undefined ? function () {
            let newTeam = {};
            newTeam.id = team_data["id"];
            teams[newTeam.id] = newTeam;
            return newTeam;
        }() : temp);
        team.name = team_data["name"];
        team.userIds = [];
        for(let i = 0; i<team_data["user-ids"].length; i++){
            team.userIds.push(team_data["user-ids"][i]);
        }
    }

    function updateTour(tour_data){
        let tourId = tour_data["id"];
        if(tourId == tour.id){

            if(tour_data["owner-id"] != undefined){
                tour.ownerId = tour_data["owner-id"];
                let id = sessionStorage.getItem('id');
                if (id != null && tour.ownerId == id) {
                    isOwner = true;
                }
            }

            if(tour_data["name"] != undefined){
                tour.name = tour_data["name"];
            }

            if(tour_data["home-id"] != undefined){
                tour.homeId = tour_data["home-id"];
            }

            if(tour_data["away-id"] != undefined){
                tour.awayId = tour_data["away-id"];
            }
            

            if(tour_data["matches"] != undefined){
                for(let i = 0; i < tour_data["matches"].length; i++){
                    let match_data = tour_data["matches"][i];
                    let matchId = match_data["id"];
                    let temp = matches[matchId];


                    let match = (temp == undefined ? function () {
                        let newMatch = {};
                        newMatch.id = matchId;

                        newMatch.playersPerTeam = match_data["players-per-team"];
                        
                        newMatch.isDouble = newMatch.playersPerTeam == 2;

                        let matchNavigator = document.createElement("a");
                        matchNavigator.href = `/ingame.html?tourid=${tourId}&matchid=${matchId}`;
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
                        
                        newMatch.row = addRowToTable(matchTable, ["", "", "", "", "", "", "", "", matchNavigator, qrDiv]);


                        
                        newMatch.playersTeam1Container = newMatch.row.cells[0];
                        let playersTeam1p = document.createElement("p");
                        playersTeam1p.innerHTML = getPlayerText(matchId, true);
                        newMatch.playersTeam1Container.appendChild(playersTeam1p);
                        let playersTeam1div = document.createElement("div");
                        newMatch.playersTeam2Container = newMatch.row.cells[1];
                        let playersTeam2p = document.createElement("p");
                        playersTeam2p.innerHTML = getPlayerText(matchId, false);
                        newMatch.playersTeam2Container.appendChild(playersTeam2p);
                        let playersTeam2div = document.createElement("div");

                        if(isOwner){

                            function sendUpdatePlayersTeam1(){
                                let playersTeam1ids = [];
                                if(playersTeam1dropDown.selectedIndex != -1){
                                    playersTeam1ids = [parseInt(playersTeam1dropDown.selectedElement.attributes["user-id"].value)];
                                }

                                if(newMatch.isDouble){
                                    if(newMatch.playersTeam1[1].selectedIndex != -1){
                                        playersTeam1ids.push(parseInt(newMatch.playersTeam1[1].selectedElement.attributes["user-id"].value));
                                    }
                                }

                                connection.send(JSON.stringify({
                                    "update": {
                                        "tours":[{
                                            "id": tour.id,
                                            "matches":[
                                                {
                                                    "id": matchId,
                                                    "players-team-1-ids":playersTeam1ids
                                                }
                                            ]
                                        }]
                                    }
                                }));
                            }

                            function sendUpdatePlayersTeam2(){
                                let playersTeam2ids = [];
                                if(playersTeam2dropDown.selectedIndex != -1){
                                    playersTeam2ids = [parseInt(playersTeam2dropDown.selectedElement.attributes["user-id"].value)];
                                }
                                if(newMatch.isDouble){
                                    if(newMatch.playersTeam2[1].selectedIndex != -1){
                                        playersTeam2ids.push(parseInt(newMatch.playersTeam2[1].selectedElement.attributes["user-id"].value));
                                    }
                                }
                                connection.send(JSON.stringify({
                                    "update": {
                                        "tours":[{
                                            "id": tour.id,
                                            "matches":[
                                                {
                                                    "id": matchId,
                                                    "players-team-2-ids":playersTeam2ids
                                                }
                                            ]
                                        }]
                                    }
                                }));
                            }


                            let playersTeam1dropDown = document.createElement("drop-down-component");
                            let playersTeam2dropDown = document.createElement("drop-down-component");

                            playersTeam1dropDown.setAttribute("placeholder", "player...");
                            playersTeam2dropDown.setAttribute("placeholder", "player...");

                            playersTeam1div.appendChild(playersTeam1dropDown);
                            playersTeam2div.appendChild(playersTeam2dropDown);
                            
                            let team1players = teams[tour.homeId].userIds;
                            let team2players = teams[tour.awayId].userIds;
                            
                            
                            
                            playersTeam1dropDown.onload = ()=>{
                                for(let u = 0; u < team1players.length; u++){
                                    let team1playerId = team1players[u];
                                    let team1player = users[team1playerId];
                                    playersTeam1dropDown.addValue(team1player.name).setAttribute("user-id", team1player.id);
                                }
                            }

                            playersTeam2dropDown.onload = ()=>{
                                for(let u = 0; u < team2players.length; u++){
                                    let team2playerId = team2players[u];
                                    let team2player = users[team2playerId];
                                    playersTeam2dropDown.addValue(team2player.name).setAttribute("user-id", team2player.id);
                                }
                            }
                            
                            newMatch.playersTeam1 = [playersTeam1dropDown];
                            newMatch.playersTeam2 = [playersTeam2dropDown];
                            
                            playersTeam1dropDown.onSelectChange = sendUpdatePlayersTeam1;
                            playersTeam2dropDown.onSelectChange = sendUpdatePlayersTeam2;

                            if(newMatch.isDouble){
                                let playersTeam1dropDown2 = document.createElement("drop-down-component");
                                let playersTeam2dropDown2 = document.createElement("drop-down-component");

                                playersTeam1dropDown2.setAttribute("placeholder", "player...");
                                playersTeam2dropDown2.setAttribute("placeholder", "player...");

                                playersTeam1div.appendChild(playersTeam1dropDown2);
                                playersTeam2div.appendChild(playersTeam2dropDown2);
                                
                                playersTeam1dropDown2.onload = ()=>{
                                    for(let u = 0; u < team1players.length; u++){
                                        let team1playerId = team1players[u];
                                        let team1player = users[team1playerId];
                                        playersTeam1dropDown2.addValue(team1player.name).setAttribute("user-id", team1player.id);
                                    }
                                }

                                playersTeam2dropDown2.onload = ()=>{
                                    for(let u = 0; u < team2players.length; u++){
                                        let team2playerId = team2players[u];
                                        let team2player = users[team2playerId];
                                        playersTeam2dropDown2.addValue(team2player.name).setAttribute("user-id", team2player.id);
                                    }
                                }
                                
                                newMatch.playersTeam1.push(playersTeam1dropDown2);
                                newMatch.playersTeam2.push(playersTeam2dropDown2);
                                
                                playersTeam1dropDown2.onSelectChange = sendUpdatePlayersTeam1;
                                playersTeam2dropDown2.onSelectChange = sendUpdatePlayersTeam2;
                            }
                           
                        }else{
                            let player1Team1 = document.createElement("p");
                            newMatch.playersTeam1 = [player1Team1];
                            playersTeam1div.appendChild(player1Team1);

                            let player1Team2 = document.createElement("p");
                            newMatch.playersTeam2 = [player1Team2];
                            playersTeam2div.appendChild(player1Team2);

                            if(newMatch.isDouble){
                                let player2Team1 = document.createElement("p");
                                newMatch.playersTeam1.push(player2Team1);
                                playersTeam1div.appendChild(player2Team1);

                                let player2Team2 = document.createElement("p");
                                newMatch.playersTeam2.push(player2Team2);
                                playersTeam2div.appendChild(player2Team2);
                            }
                        }
                        newMatch.playersTeam1Container.appendChild(playersTeam1div);
                        newMatch.playersTeam2Container.appendChild(playersTeam2div);


                        newMatch.sets = [
                            newMatch.row.cells[2], 
                            newMatch.row.cells[3], 
                            newMatch.row.cells[4], 
                            newMatch.row.cells[5], 
                            newMatch.row.cells[6]
                        ];

                        newMatch.standing = newMatch.row.cells[7];
                        

                        matches[newMatch.id] = newMatch;
                        return newMatch;
                    }() : temp);

                    if(match_data["sets"] != undefined){
                        for(let u = 0; u < match_data["sets"].length; u++){
                            let set = match_data["sets"][u];
                            match.sets[u].innerHTML = `${set["team-1-points"]} - ${set["team-2-points"]}`;
                        }
                    }

                    if(match_data["team-1-set-points"] != undefined && match_data["team-2-set-points"] != undefined){
                        match.standing.innerHTML = `${match_data["team-1-set-points"]} - ${match_data["team-2-set-points"]}`;
                    }

                    


                    function selectPlayer(dropDown, playerId){
                        dropDown.ignoreOnSelectChange = true;
                        dropDown.select(-1);
                        for(let y = 0; y < dropDown.values.length; y++){
                            let value = dropDown.values[y];
                            let valueId = parseInt(value.attributes["user-id"].value);
                            if(valueId == playerId){
                                dropDown.select(y);
                                break;
                            }
                        }
                        dropDown.ignoreOnSelectChange = false;
                    }

                    if(match_data["players-team-1-ids"] != undefined){
                        if(isOwner){
                            for(let u = 0; u < match_data["players-team-1-ids"].length; u++){
                                selectPlayer(match.playersTeam1[u], match_data["players-team-1-ids"][u]);
                            }
                        }else{
                            for(let u = 0; u < match_data["players-team-1-ids"].length; u++){
                                match.playersTeam1[u].innerHTML = users[match_data["players-team-1-ids"][u]].name;
                            }
                        }
                    }

                    if(match_data["players-team-2-ids"] != undefined){
                        if(isOwner){
                            for(let u = 0; u < match_data["players-team-2-ids"].length; u++){
                                selectPlayer(match.playersTeam2[u], match_data["players-team-2-ids"][u]);
                            }
                        }else{
                            for(let u = 0; u < match_data["players-team-2-ids"].length; u++){
                                match.playersTeam2[u].innerHTML = users[match_data["players-team-2-ids"][u]].name;
                            }
                        }
                    }

                }
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
            header.innerHTML = `Tour ${tour.id} - ${userName}`;
        }else{
            header.innerHTML = `Tour ${tour.id}`;
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
                            header.innerHTML =  `Tour ${tour.id} - ${method_data["name"]}`;
                            connection.send(JSON.stringify({
                                "get": {
                                    "users":[]
                                }
                            }));
                            break;
                        case "get":
                            for(const [data_type, data] of Object.entries(method_data)){
                                switch(data_type){
                                    case "users":
                                        for(let i = 0; i < data.length; i++){
                                            updateUser(data[i]);
                                        }
                                        connection.send(JSON.stringify({
                                            "get": {
                                                "teams":[]
                                            }
                                        }));
                                        break;
                                    case "teams":
                                        for(let i = 0; i < data.length; i++){
                                            updateTeam(data[i]);
                                        }
                                        connection.send(JSON.stringify({
                                            "get": {
                                                "tours":[{
                                                    "id": tour.id,
                                                    "matches":[]
                                                }]
                                            }
                                        }));
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
                                            updateTour(data[i]);
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