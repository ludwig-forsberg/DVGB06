const urlParams = new URLSearchParams(window.location.search);

tour = {};
match = {};

match.originalSide = true;
match.originalServe = true;
match.team1SetPoints = 0;
match.team2SetPoints = 0;
match.team1Points = 0;
match.team2Points = 0;
match.players_team_1_string = "";
match.players_team_2_string = "";


tour.id = urlParams.get("tourid");
match.id = urlParams.get("matchid");
let isOwner = false;
if (tour.id == null || tour.id == "" || match.id == null || match.id == "") {
    window.location = ".";
} else {
    users = {};

    function updateUser(user_data) {
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

    function updateTour(tour_data) {
        let tourId = tour_data["id"];
        if (tourId == tour.id) {

            if (tour_data["owner-id"] != undefined) {
                tour.ownerId = tour_data["owner-id"];
                let id = sessionStorage.getItem('id');
                if (id != null && tour.ownerId == id) {
                    isOwner = true;
                }
                setActionUI();
            }

            if (tour_data["name"] != undefined) {
                tour.name = tour_data["name"];
            }

            if (tour_data["home-id"] != undefined) {
                tour.homeId = tour_data["home-id"];
            }

            if (tour_data["away-id"] != undefined) {
                tour.awayId = tour_data["away-id"];
            }


            if (tour_data["matches"] != undefined) {
                for (let i = 0; i < tour_data["matches"].length; i++) {
                    let match_data = tour_data["matches"][i];
                    let matchId = match_data["id"];

                    if (matchId == match.id) {

                        if (match_data["original-side"] != undefined) match.originalSide = match_data["original-side"];
                        if (match_data["original-serve"] != undefined) match.originalServe = match_data["original-serve"];

                        if (match_data["team-1-set-points"] != undefined) match.team1SetPoints = match_data["team-1-set-points"];
                        if (match_data["team-2-set-points"] != undefined) match.team2SetPoints = match_data["team-2-set-points"];

                        if (match_data["sets"] != undefined) {
                            let sets = match_data["sets"];
                            match.team1Points = sets[sets.length - 1]["team-1-points"];
                            match.team2Points = sets[sets.length - 1]["team-2-points"];
                        }



                        if (match_data["players-team-1-ids"] != undefined) {
                            let players_team_1_string = "";
                            for (let u = 0; u < match_data["players-team-1-ids"].length; u++) {
                                if (players_team_1_string != "") players_team_1_string += "/";
                                players_team_1_string += " " + users[match_data["players-team-1-ids"][u]].name + " "
                            }
                            match.players_team_1_string = players_team_1_string;
                        }


                        if (match_data["players-team-2-ids"] != undefined) {
                            let players_team_2_string = "";
                            for (let u = 0; u < match_data["players-team-2-ids"].length; u++) {
                                if (players_team_2_string != "") players_team_2_string += "/";
                                players_team_2_string += " " + users[match_data["players-team-2-ids"][u]].name + " "
                            }
                            match.players_team_2_string = players_team_2_string;
                        }
                    }
                }
                updateUI();
            }
        }
    }

    window.addEventListener('load', (event) => {
        var wsproto = (location.protocol === 'https:') ? 'wss:' : 'ws:';
        connection = new WebSocket(wsproto + '//' + window.location.host + '/websocket');

        let userId = sessionStorage.getItem('id');
        let userName = sessionStorage.getItem('name');
        let authenticated = userId != null;
        if (authenticated) {
            userId = parseInt(userId);
        }

        connection.onmessage = function (e) {
            if (e.data != "") {
                let json = JSON.parse(e.data);
                console.log(json);
                for (const [method, method_data] of Object.entries(json)) {
                    switch (method) {
                        case "auth":
                            sessionStorage.setItem('id', method_data["id"]);
                            sessionStorage.setItem('name', method_data["name"]);
                            connection.send(JSON.stringify({
                                "get": {
                                    "users": []
                                }
                            }));
                            break;
                        case "get":
                            for (const [data_type, data] of Object.entries(method_data)) {
                                switch (data_type) {
                                    case "users":
                                        for (let i = 0; i < data.length; i++) {
                                            updateUser(data[i]);
                                        }
                                        connection.send(JSON.stringify({
                                            "get": {
                                                "tours": [{
                                                    "id": tour.id,
                                                    "matches": [{
                                                        "id": match.id
                                                    }]
                                                }]
                                            }
                                        }));
                                        break;
                                    case "teams":
                                        break;
                                    case "tours":
                                        for (let i = 0; i < data.length; i++) {
                                            updateTour(data[i]);
                                        }
                                        break;
                                }
                            }
                            break;
                        case "update":
                            for (const [data_type, data] of Object.entries(method_data)) {
                                switch (data_type) {
                                    case "users":
                                        for (let i = 0; i < data.length; i++) {
                                        }
                                        break;
                                    case "teams":
                                        for (let i = 0; i < data.length; i++) {
                                        }
                                        break;
                                    case "tours":
                                        for (let i = 0; i < data.length; i++) {
                                            updateTour(data[i]);
                                        }
                                        break;
                                }
                            }
                            break;
                        case "create":
                            for (const [data_type, data] of Object.entries(method_data)) {
                                switch (data_type) {
                                    case "teams":
                                        for (let i = 0; i < data.length; i++) {
                                        }
                                        break;
                                    case "tours":
                                        for (let i = 0; i < data.length; i++) {
                                        }
                                        break;
                                }
                            }
                            break;
                        case "delete":
                            for (const [data_type, data] of Object.entries(method_data)) {
                                switch (data_type) {
                                    case "teams":
                                        for (let i = 0; i < data.length; i++) {
                                        }
                                        break;
                                    case "tours":
                                        for (let i = 0; i < data.length; i++) {
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

        if (!authenticated)
            setActionUI();
    });
}

function team1AddPoint() {
    connection.send(JSON.stringify({
        "update": {
            "tours": [{
                "id": tour.id,
                "matches": [{
                    "id": match.id,
                    "actions": ["team-1-add-point"]
                }]
            }]
        }
    }));
}

function team1SubtractPoint() {
    connection.send(JSON.stringify({
        "update": {
            "tours": [{
                "id": tour.id,
                "matches": [{
                    "id": match.id,
                    "actions": ["team-1-subtract-point"]
                }]
            }]
        }
    }));
}

function team2AddPoint() {
    connection.send(JSON.stringify({
        "update": {
            "tours": [{
                "id": tour.id,
                "matches": [{
                    "id": match.id,
                    "actions": ["team-2-add-point"]
                }]
            }]
        }
    }));
}

function team2SubtractPoint() {
    connection.send(JSON.stringify({
        "update": {
            "tours": [{
                "id": tour.id,
                "matches": [{
                    "id": match.id,
                    "actions": ["team-2-subtract-point"]
                }]
            }]
        }
    }));
}

var actionUIisSet = false;
function setActionUI() {
    if (actionUIisSet)
        return;
    actionUIisSet = true;
    if (isOwner) {




        p1_add_point.onclick = () => {
            if (match.originalSide)
                team1AddPoint();
            else
                team2AddPoint();
        };

        p1_sub_point.onclick = () => {
            if (match.originalSide)
                team1SubtractPoint();
            else
                team2SubtractPoint();
        };

        p2_add_point.onclick = () => {
            if (match.originalSide)
                team2AddPoint();
            else
                team1AddPoint();
        };

        p2_sub_point.onclick = () => {
            if (match.originalSide)
                team2SubtractPoint();
            else
                team1SubtractPoint();
        };

        reset.onclick = () => {
            connection.send(JSON.stringify({
                "update": {
                    "tours": [{
                        "id": tour.id,
                        "matches": [{
                            "id": match.id,
                            "actions": ["reset"]
                        }]
                    }]
                }
            }));
        };

        swap_sides.onclick = () => {
            connection.send(JSON.stringify({
                "update": {
                    "tours": [{
                        "id": tour.id,
                        "matches": [{
                            "id": match.id,
                            "actions": ["swap-sides"]
                        }]
                    }]
                }
            }));
        };

        swap_serve.onclick = () => {
            connection.send(JSON.stringify({
                "update": {
                    "tours": [{
                        "id": tour.id,
                        "matches": [{
                            "id": match.id,
                            "actions": ["swap-serve"]
                        }]
                    }]
                }
            }));
        };
        action_buttons.style.display = "";
        score_controller_buttons.style.display = "";
    } else {
        action_buttons.remove();
        score_controller_buttons.remove();
    }
}

function updateUI() {
    if (match.originalSide) {
        text_p1_set.innerHTML = match.team1SetPoints;
        text_p2_set.innerHTML = match.team2SetPoints;
        text_p1_points.innerHTML = match.team1Points;
        text_p2_points.innerHTML = match.team2Points;
        score_container_p1.classList.remove("blue");
        score_container_p1.classList.add("red");
        score_container_p2.classList.remove("red");
        score_container_p2.classList.add("blue");
    } else {
        text_p1_set.innerHTML = match.team2SetPoints;
        text_p2_set.innerHTML = match.team1SetPoints;
        text_p1_points.innerHTML = match.team2Points;
        text_p2_points.innerHTML = match.team1Points;
        score_container_p2.classList.remove("blue");
        score_container_p2.classList.add("red");
        score_container_p1.classList.remove("red");
        score_container_p1.classList.add("blue");
    }

    let p1_serves = true;

    if (!match.originalSide) p1_serves = !p1_serves;
    if (!match.originalServe) p1_serves = !p1_serves;
    if (Math.floor((match.team1Points + match.team2Points) / 2) % 2 == 1) p1_serves = !p1_serves;
    if ((match.team1SetPoints + match.team2SetPoints) % 2 == 1) p1_serves = !p1_serves;


    if (p1_serves) {
        p1_serve_indicator.style.visibility = "";
        p2_serve_indicator.style.visibility = "hidden";
    } else {
        p2_serve_indicator.style.visibility = "";
        p1_serve_indicator.style.visibility = "hidden";
    }

    setPlayerNames(match.players_team_1_string, true);
    setPlayerNames(match.players_team_2_string, false);

    if (speaker_toggle.checked) {
        if (use_recorded_audio_toggle.checked) {
            var audioList = [];

            function addAudioToList(str) {
                let audio = recordedAudio[str];
                if (audio == undefined) {
                    console.log("Audio not found: ", str);
                } else {
                    audioList.push(audio);
                }
                console.log(audio)
            }


            if ((p1_serves && match.originalSide) || (!p1_serves && !match.originalSide)) {
                if (match.team1Points <= 11 && match.team2Points <= 11) {
                    if (match.originalSide) {
                        addAudioToList("" + match.team1Points);
                        addAudioToList("" + match.team2Points);
                    } else {
                        addAudioToList("" + match.team2Points);
                        addAudioToList("" + match.team1Points);
                    }
                }
                addAudioToList("player1");
            } else {
                if (match.team1Points <= 11 && match.team2Points <= 11) {
                    if (match.originalSide) {
                        addAudioToList("" + match.team2Points);
                        addAudioToList("" + match.team1Points);
                    } else {
                        addAudioToList("" + match.team1Points);
                        addAudioToList("" + match.team2Points);
                    }
                }
                addAudioToList("player2");
            }
            addAudioToList("serves");
            if ((match.team1Points >= 10 || match.team2Points >= 10) && match.team1Points != match.team2Points)
                addAudioToList("setpoint");

            function playAudio() {
                let audio = audioList.shift();
                if (audio != undefined)
                    audio.play();
                if (audioList.length > 0) {
                    audio.onended = () => {
                        playAudio();
                    }
                }
            }
            if (audioList.length > 0)
                playAudio();
        } else {
            messageParts = [];

            if (p1_serves) {
                messageParts.push(match.team1Points + " " + match.team2Points);
                messageParts.push(team_1_players_p.innerHTML.replaceAll("/", "and") + " serves");
            } else {
                messageParts.push(match.team2Points + " " + match.team1Points);
                messageParts.push(team_2_players_p.innerHTML.replaceAll("/", "and") + " serves");

            }

            if ((match.team1Points >= 10 || match.team2Points >= 10) && match.team1Points != match.team2Points)
                messageParts.push("setpoint");

            messageParts = messageParts.join(". ");

            const speak = (textToSpeak) => {
                const msg = new SpeechSynthesisUtterance();
                const voices = window.speechSynthesis.getVoices();
                msg.voice = voices[0];
                msg.volume = 1; // 0 to 1
                msg.rate = 1.5; // 0.1 to 10
                msg.pitch = .15; // 0 to 2
                msg.text = textToSpeak;
                msg.lang = 'en-US';

                speechSynthesis.speak(msg);
            }
            speak(messageParts);

        }
    }
}

function setPlayerNames(value, team1) {
    if (team1) {
        if (match.originalSide) {
            team_1_players_p.innerHTML = (value == "" ? "Team 1" : value);
        } else {
            team_2_players_p.innerHTML = (value == "" ? "Team 1" : value);
        }
    } else {
        if (match.originalSide) {
            team_2_players_p.innerHTML = (value == "" ? "Team 2" : value);
        } else {
            team_1_players_p.innerHTML = (value == "" ? "Team 2" : value);
        }
    }
}

var recordedAudio = {};
var requiredAudio = [
    "player1",
    "player2",
    "serves",
    "setpoint",
    "0",
    "1",
    "2",
    "3",
    "4",
    "5",
    "6",
    "7",
    "8",
    "9",
    "10",
    "11"
];