(function () {

    var _stopRecTimeout2 = 500;
    var _threshold2 = -70; // voice dB
    var _harkInterval2 = 100;
    var _bufferSize = 8;

    var numChannels = 1;

    var magnitudes = [];
    var magnitude_threshold = 0.005;

    var recording_input = false;
    var recordAudioCounter = 0;
    var recordingLength = 0;
    var speaking = false;

    var channel = [];
    var source;

    var trained = false;
    var currentTrainingIndex = null;

    var speechNode = null;

    var speechEvents = null;

    async function startVoice() {
        if (recording_input)
            return;
        recording_input = true;
        channel.length = 0;
        recordAudioCounter = 0;
        const stream = await navigator.mediaDevices.getUserMedia({ audio: true });

        if (!trained) {
            popup.style.display = "flex";
            popup_header.innerHTML = "Say the following word:";
            if(match.originalSide) popup_text.innerHTML = team_1_players_p.innerHTML;
            else popup_text.innerHTML = team_2_players_p.innerHTML;
        }


        var audioContextType = window.AudioContext || window.webkitAudioContext;
        var track = stream.getTracks()[0];
        // create the MediaStreamAudioSourceNode
        // Setup Audio Context
        context = new audioContextType();
        source = context.createMediaStreamSource(stream);

        // create a ScriptProcessorNode
        if (!context.createScriptProcessor) {
            speechNode = context.createJavaScriptNode(Recognize.bufferSize, numChannels, numChannels);
        } else {
            speechNode = context.createScriptProcessor(Recognize.bufferSize, numChannels, numChannels);
        }

        // listen to the audio data, and record into the buffer, this is important to catch the fraction of second before the speech started.
        speechNode.onaudioprocess = (e) => {
            var left = e.inputBuffer.getChannelData(0);

            if (channel.length < _bufferSize || speaking) {
                channel.push(new Float32Array(left));
                recordingLength += Recognize.bufferSize;
            }
            else {
                channel.splice(0, 1);
                channel.push(new Float32Array(left));
            }
        }

        // connect the ScriptProcessorNode with the input audio
        source.connect(speechNode);
        speechNode.connect(context.destination);

        speechEvents = hark(stream, { interval: _harkInterval2, threshold: _threshold2, play: false, recoredInterval: _stopRecTimeout2 });

        speechEvents.on('speaking', function () {
            speaking = true;
            console.log('speaking');
            popup_container.style.background = "Green";
        });

        speechEvents.on('stopped_speaking', function () {
            console.log('stopped speaking');
            speaking = false;
            popup_container.style.background = "White";

            console.log(channel.length);

            magnitudes = [];
            for (var i = 0; i < channel.length; i++) {
                magnitudes.push(Math.max(...channel[i]));
            }

            for (var i = channel.length - 1; i >= 0; i--) {
                if (magnitudes[i] < magnitude_threshold) {
                    channel.pop();
                    recordingLength -= Recognize.bufferSize;
                } else {
                    break;
                }
            }

            var blob = Utils.bufferToBlob(channel, recordingLength);

            Utils.getVoiceFile(blob, 0);

            var reader = new window.FileReader();
            reader.readAsDataURL(blob);

            // read the blob and start processing according to the system state (trained or not)
            var _channel = channel.slice(0);
            reader.onloadend = () => {
                if (trained) {
                    let result = Recognize.recognize(_channel, () => {

                    });
                    if (result) {
                        console.log("Great! the result is ===> " + result.transcript + " <=== try more.");
                        result.transcript();
                    }
                    else {
                        console.log("Didn't Got it! please try to Again loud and clear.");
                    }
                    console.log(result);
                }
                else {
                    let success = Recognize.train(_channel, Recognize.dictionary[currentTrainingIndex % Recognize.dictionary.length], () => {

                    });
                    traingNextWord(success);

                }
            }

            channel.length = 0;
            recordingLength = 0;
            recordAudioCounter++;
        });

    };

    traingNextWord = (success) => {
        if (success) {
            // next word
            let i = currentTrainingIndex + 1;
            if (i > Recognize.dictionary.length * 2 - 1) {
                trained = true;
                currentTrainingIndex = i;
                console.log("training is finished, now we will try to guess what you are trying to say from the trained vocabulary.");
                console.log("recognizing mode");
                popup.style.display = "none";
            }
            else {
                currentTrainingIndex = i;
                if (i % 2 == 0) {
                    if(match.originalSide) popup_text.innerHTML = team_1_players_p.innerHTML;
                    else popup_text.innerHTML = team_2_players_p.innerHTML;
                } else {
                    if(match.originalSide) popup_text.innerHTML = team_2_players_p.innerHTML;
                    else popup_text.innerHTML = team_1_players_p.innerHTML;
                }
                console.log("Good! say the next word loud and clear, and wait until we process it.  ===>  " + Recognize.dictionary[i % Recognize.dictionary.length]);
            }
        }
        else {
            console.log("we didn't got it, try again, say the next word loud and clear, and wait until we process it.    " + Recognize.dictionary[currentTrainingIndex % Recognize.dictionary.length]);
        }
    }

    function stopVoice() {
        recording_input = false;
        if (speechEvents != null) {
            speechEvents.stop();
            speechEvents = null;
        }
        if (speechNode != null) {
            speechNode.disconnect();
            speechNode = null;
        }
    }

    window.addEventListener('load', (event) => {
        start_voice_button.onclick = startVoice;
        stop_voice_button.onclick = stopVoice;
    });
})()