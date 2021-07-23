var connection;

function addTeam() {
    if (addTeamName.value != "") {
        addTeamContainer.style.display = "none";
        connection.send(JSON.stringify({
            "create": {
                "teams":[
                    {
                        "name": addTeamName.value,
                        "owner-id": parseInt(sessionStorage.getItem('id'))
                    }
                ]
            }
        }));
        addTeamName.value = "";
        addTeamButton.classList.add("disabled");
    }
}

function addTour() {
    if (addTourName.value != "" && addTourHome.selectedIndex != -1 && addTourAway.selectedIndex != -1) {
        addTourContainer.style.display = "none";
        connection.send(JSON.stringify({
            "create": {
                "tours":[
                    {
                        "name": addTourName.value,
                        "home-id": addTourHome.selectedElement.id,
                        "away-id": addTourAway.selectedElement.id,
                        "owner-id": parseInt(sessionStorage.getItem('id'))
                    }
                ]
            }
        }));
        addTourName.value = "";
        addTourButton.classList.add("disabled");
        addTourHome.select(-1);
        addTourAway.select(-1);
    }
}


function showAddTeamPopup() {
    addTeamContainer.style.display = "flex";
}

function showTeamUsers(team){
    selectedTeam = team;
    setTeamUsersUI();
    teamUsersContainer.style.display = "flex";
}

function showAddTourPopup() {
    addTourContainer.style.display = "flex";
    setAddTourUI();
}

function closeAllPopups(){
    addTeamContainer.style.display =
    addTourContainer.style.display =
    teamUsersContainer.style.display = "none";
    selectedTeam = null;
}

window.addEventListener("click", function (event) {
    if (event.target == addTeamContainer) {
        addTeamContainer.style.display = "none";
    } else if (event.target == addTourContainer) {
        addTourContainer.style.display = "none";
    } else if (event.target == teamUsersContainer) {
        teamUsersContainer.style.display = "none";
        selectedTeam = null;
    }
});

function login() {
    if (loginInput.value != "") {
        loginContainer.style.display = "none";

        connection.send(JSON.stringify({
            "auth": {
                "name": loginInput.value
            }
        }));
    }
}


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

function findRowIndex(table, row){
    for(let i = 0; i < table.rows.length; i++){
        if(table.rows[i] == row){
            return i;
        }
    }
    return -1;
}

function clearTableData(table){
    for(let i = table.rows.length-1; i >= 1; i--){
        table.deleteRow(i);
    }
}

var users = {};
var teams = {};
var tours = {};

function checkAddTeamButton() {
    if (addTeamName.value == "") {
        addTeamButton.classList.add("disabled");
    } else {
        addTeamButton.classList.remove("disabled");
    }
}

function checkAddTourButton() {
    if (addTourName.value == "" || addTourHome.selectedIndex == -1 || addTourAway.selectedIndex == -1) {
        addTourButton.classList.add("disabled");
    } else {
        addTourButton.classList.remove("disabled");
    }
}

var selectedTeam = null;
function setTeamUsersUI(){
    let id = sessionStorage.getItem('id');
    let owner = (selectedTeam.owner.id == parseInt(id));
    
    teamUsersHeader.innerHTML = selectedTeam.name;
    clearTableData(teamUsersTable);

    if(owner){
        for(const [id, user] of Object.entries(users)){
            let userId = user.id;
            let teamId = selectedTeam.id;
    
            if(user.teamId == -1){
    
    
                let addButton = document.createElement("button-component");
                addButton.innerHTML = "+";
                addButton.classList.add("green");
                addButton.onclick = ()=>{
                    connection.send(JSON.stringify({
                        "update":{
                            "users":[{
                                "id": userId,
                                "team-id": teamId
                            }]
                        }
                    }));
                };
                addRowToTable(teamUsersTable, [user.name, addButton])
            }else if(user.teamId == selectedTeam.id){
                
    
                let removeButton = document.createElement("button-component");
                removeButton.innerHTML = "-";
                removeButton.classList.add("red");
                removeButton.onclick = ()=>{
                    connection.send(JSON.stringify({
                        "update":{
                            "users":[{
                                "id": userId,
                                "team-id": -1
                            }]
                        }
                    }));
                };
                addRowToTable(teamUsersTable, [user.name, removeButton])
            }
        }
    }else{
        for(const [id, user] of Object.entries(users)){
            if(user.teamId == selectedTeam.id){
                addRowToTable(teamUsersTable, [user.name])
            }
        }
    }
    
}

function setAddTourUI(){
    addTourHome.clearValues();
    addTourAway.clearValues();

    for(const [id, team] of Object.entries(teams)){
        let valueHome = document.createElement("value");
        valueHome.innerHTML = team.name;
        valueHome.id = team.id;
        let valueAway = document.createElement("value");
        valueAway.innerHTML = team.name;
        valueAway.id = team.id;
        addTourHome.addElement(valueHome);
        addTourAway.addElement(valueAway);
    }

    checkAddTourButton();
}

function updateUser(user_data){
    let temp = users[user_data["id"]];
    let teamId = user_data["team-id"];
    let user = (temp == undefined ? function () {
        let newUser = {};
        newUser.id = user_data["id"];
        newUser.row = addRowToTable(userTable, [user_data["name"], ""])
        users[newUser.id] = newUser;
        return newUser;
    }() : temp);
    user.name = user_data["name"];
    user.teamId = user_data["team-id"];
    let teamName = "";
    if(teamId == -1){
        teamName = "none";
    }else if(teams[teamId] == undefined){
        teamName = "not found";
    }else{
        teamName = teams[teamId].name;
    }
    user.row.cells[1].innerHTML = teamName;
    if(teamUsersContainer.style.display == "flex"){
        setTeamUsersUI();
    }
}

function updateTeam(team_data){
    let temp = teams[team_data["id"]];
    let team = (temp == undefined ? function () {
        let newTeam = {};
        newTeam.id = team_data["id"];
        newTeam.name = team_data["name"];
        newTeam.owner = users[team_data["owner-id"]];
        
        let id = sessionStorage.getItem('id');

        if(id != null && newTeam.owner.id == parseInt(id)){
            let editButton = document.createElement("button-component");
            editButton.innerHTML = "Edit";
            editButton.setAttribute("icon", "edit");
            editButton.onclick = ()=>{
                showTeamUsers(newTeam);
            };
            let removeButton = document.createElement("button-component");
            removeButton.innerHTML = "-";
            removeButton.classList.add("red");
            removeButton.onclick = ()=>{
                connection.send(JSON.stringify({
                    "delete": {
                        "teams":[{
                            "id": newTeam.id
                        }]
                    }
                }));
            };
            newTeam.row = addRowToTable(teamTable, [newTeam.name, newTeam.owner.name, editButton, removeButton])
        }else{
            let showUsersButton = document.createElement("button-component");
            showUsersButton.innerHTML = "Users";
            showUsersButton.setAttribute("icon", "users");
            showUsersButton.onclick = ()=>{
                showTeamUsers(newTeam);
            };
            newTeam.row = addRowToTable(teamTable, [newTeam.name, newTeam.owner.name, showUsersButton])
        }

        teams[newTeam.id] = newTeam;
        return newTeam;
    }() : temp);

    for(const [id, user] of Object.entries(users)){
        if(user.teamId == team.id){
            user.row.cells[1].innerHTML = team.name;
        }
    } 
}

function updateTour(tour_data){
    let id = sessionStorage.getItem('id');
    let owner = (tour_data["owner-id"] == parseInt(id));

    let temp = tours[tour_data["id"]];
    let team = (temp == undefined ? function () {
        let newTour = {};
        let tourId = tour_data["id"];
        newTour.id = tourId;

        newTour.name = tour_data["name"];
        newTour.home = teams[tour_data["home-id"]];
        newTour.away = teams[tour_data["away-id"]];
        newTour.owner = users[tour_data["owner-id"]];

        let tourNavigator = document.createElement("a");
        tourNavigator.href = "/tour.html?id="+tourId;
        tourNavigator.innerHTML = "link";

        let qrDiv = document.createElement("div");
        let qrToggleButton = document.createElement("button-component");
        qrToggleButton.innerHTML = "Show";
        let qrCodeContainer = document.createElementNS("http://www.w3.org/2000/svg", "svg");
        new QRCode(qrCodeContainer, tourNavigator.href);
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

        if(owner){
            let removeButton = document.createElement("button-component");
            removeButton.innerHTML = "-";
            removeButton.classList.add("red");
            removeButton.onclick = ()=>{
                connection.send(JSON.stringify({
                    "delete": {
                        "tours":[{
                            "id": tourId
                        }]
                    }
                }));
            };
            newTour.row = addRowToTable(tourTable, [newTour.name, newTour.home.name, newTour.away.name, newTour.owner.name, tourNavigator, qrDiv, removeButton])
        }else{

            newTour.row = addRowToTable(tourTable, [newTour.name, newTour.home.name, newTour.away.name, newTour.owner.name, tourNavigator, qrDiv])
        }

        tours[newTour.id] = newTour;
        return newTour;
    }() : temp);
}

function removeTeamUI(team_data) {
    let team = teams[team_data["id"]];
    if(team != undefined){
        let index = findRowIndex(teamTable, team.row);
        if(index != -1)
            teamTable.deleteRow(index);
        delete teams[team_data["id"]];
    }
}

function removeTourUI(tour_data) {
    let tour = tours[tour_data["id"]];
    if(tour != undefined){
        let index = findRowIndex(tourTable, tour.row);
        if(index != -1)
            tourTable.deleteRow(index);

        delete tours[tour_data["id"]];
    }
}

window.addEventListener('load', (event) => {
    var wsproto = (location.protocol === 'https:') ? 'wss:' : 'ws:';
    connection = new WebSocket(wsproto + '//' + window.location.host + '/websocket');

    connection.onmessage = function (e) {
        if (e.data != "") {
            let json = JSON.parse(e.data);
            console.log(json);
            for(const [method, method_data] of Object.entries(json)){
                switch(method){
                    case "auth":
                        sessionStorage.setItem('id', method_data["id"]);
                        sessionStorage.setItem('name', method_data["name"]);
                        header.innerHTML = method_data["name"];
                        connection.send(JSON.stringify({
                            "get": {
                                "users":[],
                                "teams":[],
                                "tours":[]
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
                                    break;
                                case "teams":
                                    for(let i = 0; i < data.length; i++){
                                        updateTeam(data[i]);
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
                                        updateUser(data[i]);
                                    }
                                    break;
                                case "teams":
                                    for(let i = 0; i < data.length; i++){
                                        updateTeam(data[i]);
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
                                        updateTeam(data[i]);
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
                    case "delete":
                        for(const [data_type, data] of Object.entries(method_data)){
                            switch(data_type){
                                case "teams":
                                    for(let i = 0; i < data.length; i++){
                                        removeTeamUI(data[i]);
                                    }
                                    break;
                                case "tours":
                                    for(let i = 0; i < data.length; i++){
                                        removeTourUI(data[i]);
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

    let id = sessionStorage.getItem('id');
    if (id != null) {
        loginContainer.style.display = "none";
    } else {
        loginContainer.style.display = "flex";
    }


    connection.onopen = function (e) {
        if (id != null) {
            connection.send(JSON.stringify({
                "auth": {
                    "id": parseInt(id),
                    "name": sessionStorage.getItem('name')
                }
            }));
        }
    }


    loginButton.classList.add("disabled");

    loginInput.oninput = () => {
        if (loginInput.value == "") {
            loginButton.classList.add("disabled");
        } else {
            loginButton.classList.remove("disabled");
        }
    }

    loginInput.addEventListener("keyup", function(event) {
        if (event.keyCode === 13) {
          event.preventDefault();
          login();
        }
    });

    checkAddTourButton();
    checkAddTeamButton();
    addTourName.onValueChange = checkAddTourButton;
    addTourHome.onSelectChange = (selectedIndex)=>{
        for(let i = 0; i < addTourAway.values.length; i++){
            addTourAway.values[i].style.display = "";
        }
        if(selectedIndex != -1)
            addTourAway.values[selectedIndex].style.display = "none";
        checkAddTourButton();
    };
    addTourAway.onSelectChange = (selectedIndex)=>{
        for(let i = 0; i < addTourHome.values.length; i++){
            addTourHome.values[i].style.display = "";
        }
        if(selectedIndex != -1)
            addTourHome.values[selectedIndex].style.display = "none";
        checkAddTourButton();
    };
    addTeamName.onValueChange = checkAddTeamButton;

    addTeamName.addEventListener("keyup", function(event) {
        if (event.keyCode === 13) {
          event.preventDefault();
          addTeam();
        }
    });

    addTourName.addEventListener("keyup", function(event) {
        if (event.keyCode === 13) {
          event.preventDefault();
          addTour();
        }
    });
});



