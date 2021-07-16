var connection;

function login() {
    if (loginInput.value != "") {

        //sessionStorage.setItem('name', loginInput.value);
        loginContainer.style.display = "none";



        connection.send(JSON.stringify({
            "actions": [{
                "action": 1,
                "data": {
                    "name": loginInput.value
                }
            }]
        }));
    }
}


function addTeam() {
    if (addTeamName.value != "") {
        addTeamContainer.style.display = "none";
        connection.send(JSON.stringify({
            "actions": [{
                "action": 5,
                "data": {
                    "name": addTeamName.value,
                    "owner-id": parseInt(sessionStorage.getItem('id'))
                }
            }]
        }));
        addTeamName.value = "";
        addTeamButton.classList.add("disabled");
    }
}

function addTour() {
    if (addTourName.value != "" && addTourHome.selectedIndex != -1 && addTourAway.selectedIndex != -1) {
        addTourContainer.style.display = "none";
        connection.send(JSON.stringify({
            "actions": [{
                "action": 6,
                "data": {
                    "name": addTourName.value,
                    "home-id": addTourHome.selectedElement.id,
                    "away-id": addTourAway.selectedElement.id,
                    "owner-id": parseInt(sessionStorage.getItem('id'))
                }
            }]
        }));
        addTourName.value = "";
        addTourHome.select(-1);
        addTourAway.select(-1);
    }
}

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

function addRowToTable(table, data) {
    let row = table.insertRow(1);

    for (let i = 0; i < data.length; i++) {
        let cell = row.insertCell(i);
        if(typeof(data[i]) == "object"){
            cell.appendChild(data[i]);
        }else{

            cell.innerHTML = data[i];
        }
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

var users = {};
var teams = {};
var tours = {};

function showAddTeamPopup() {
    addTeamContainer.style.display = "flex";
}





function showAddTourPopup() {
    addTourContainer.style.display = "flex";
}



window.addEventListener("click", function (event) {
    if (event.target == addTeamContainer) {
        addTeamContainer.style.display = "none";
    } else if (event.target == addTourContainer) {
        addTourContainer.style.display = "none";
    }
});

function addUserUI(userData) {
    let temp = users[userData["id"]];
    let user = (temp == undefined ? function () {
        let newUser = {};
        newUser.id = userData["id"];
        newUser.row = addRowToTable(userTable, [userData["name"], userData["team"]])
        users[newUser.id] = newUser;
        return newUser;
    }() : temp);
    user.name = userData["name"];
    user.team = userData["team"];
}

function addTeamUI(teamData) {
    let temp = teams[teamData["id"]];
    let team = (temp == undefined ? function () {
        let newTeam = {};
        newTeam.id = teamData["id"];
        newTeam.name = teamData["name"];
        newTeam.owner = users[teamData["owner"]["id"]];
        
        let id = sessionStorage.getItem('id');

        if(id != null && newTeam.owner.id == parseInt(id)){

            let removeButton = document.createElement("button-component");
            removeButton.innerHTML = "-";
            removeButton.onclick = ()=>{
                connection.send(JSON.stringify({
                    "actions": [{
                        "action": 7,
                        "data": {
                            "id": newTeam.id
                        }
                    }]
                }));
            };
            newTeam.row = addRowToTable(teamTable, [newTeam.name, newTeam.owner.name, removeButton])
        }else{
            newTeam.row = addRowToTable(teamTable, [newTeam.name, newTeam.owner.name])
        }

        teams[newTeam.id] = newTeam;
        return newTeam;
    }() : temp);


    let valueHome = document.createElement("value");
    valueHome.innerHTML = team.name;
    valueHome.id = team.id;
    let valueAway = document.createElement("value");
    valueAway.innerHTML = team.name;
    valueAway.id = team.id;
    addTourHome.addElement(valueHome);
    addTourAway.addElement(valueAway);

}

function addTourUI(tourData) {
    let temp = tours[tourData["id"]];
    let team = (temp == undefined ? function () {
        let newTour = {};
        newTour.id = tourData["id"];

        newTour.name = tourData["name"];
        newTour.home = teams[tourData["home"]["id"]];
        newTour.away = teams[tourData["away"]["id"]];
        newTour.owner = users[tourData["owner"]["id"]];

        newTour.row = addRowToTable(tourTable, [newTour.name, newTour.home.name, newTour.away.name, newTour.owner.name])
        tours[newTour.id] = newTour;
        return newTour;
    }() : temp);
}

function removeTeamUI(teamData) {
    let team = teams[teamData["id"]];
    if(team != undefined){
        let index = findRowIndex(teamTable, team.row);

        console.log("INdex: " + index);
        if(index != -1)
            teamTable.deleteRow(index);

        delete teams[teamData["id"]];
    }
}

window.addEventListener('load', (event) => {
    var wsproto = (location.protocol === 'https:') ? 'wss:' : 'ws:';
    connection = new WebSocket(wsproto + '//' + window.location.host + '/websocket');

    connection.onmessage = function (e) {
        if (e.data != "") {
            console.log(e.data);
            let json = JSON.parse(e.data);
            console.log(json);
            let actions = json["actions"];
            for(let index = 0; index < actions.length; index++){

                let action = actions[index]["action"];
                let data = actions[index]["data"];
                switch (action) {
                    case 1:
                        sessionStorage.setItem('id', data["id"]);
                        sessionStorage.setItem('name', data["name"]);
                        console.log(data["id"])
                        header.innerHTML = data["name"];
                        break;
                    case 2:
                        if (data["users"] != undefined)
                            for (let i = 0; i < data["users"].length; i++) {
                                addUserUI(data["users"][i]);
                            }
                        break;
                    case 3:
                        if (data["teams"] != undefined)
                            for (let i = 0; i < data["teams"].length; i++) {
                                addTeamUI(data["teams"][i]);
                            }
                        break;
                    case 4:
                        if (data["tours"] != undefined)
                            for (let i = 0; i < data["tours"].length; i++) {
                                addTourUI(data["tours"][i]);
                            }
                        break;
                    case 7:
                        if (data["teams"] != undefined)
                            for (let i = 0; i < data["teams"].length; i++) {
                                removeTeamUI(data["teams"][i]);
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
                "actions": [{
                    "action": 1,
                    "data": {
                        "id": parseInt(id),
                        "name": sessionStorage.getItem('name')
                    }
                }]
            }));
        }

    }



    if (loginInput.value == "") {
        loginButton.classList.add("disabled");
    }

    loginInput.oninput = () => {
        if (loginInput.value == "") {
            loginButton.classList.add("disabled");
        } else {
            loginButton.classList.remove("disabled");
        }
    }

    checkAddTourButton();
    checkAddTeamButton();
    addTourName.onValueChange = addTourHome.onSelectChange = addTourAway.onSelectChange = checkAddTourButton;
    addTeamName.onValueChange = checkAddTeamButton;
});



