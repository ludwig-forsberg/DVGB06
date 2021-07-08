

(function () {
  var _stopRecTimeout2 = 500;
  var _threshold2 = -70; // voice dB
  var _harkInterval2 = 100;
  var _bufferSize = 8;

  var numChannels = 1;


  var recording_output = false;
  var recordAudioCounter = 0;
  var recordingLength = 0;
  var audioName = "";
  var speaking = false;


  var magnitudes = [];
  var magnitude_threshold = 0.005;

  function setAudioName() {
    audioName = requiredAudio[recordAudioCounter];
    let text = "";
    if (audioName == "player1") {
      if (original_side) {
        if (input_p1_name.value == "" || input_p1_name.value == "Player 1") {
          text = "Player 1";
        } else {
          text = input_p1_name.value + " (Player 1)";
        }
      } else {
        if (input_p2_name.value == "" || input_p2_name.value == "Player 1") {
          text = "Player 1";
        } else {
          text = input_p2_name.value + " (Player 1)";
        }
      }
    } else if (audioName == "player2") {
      if (original_side) {
        if (input_p2_name.value == "" || input_p2_name.value == "Player 2") {
          text = "Player 2";
        } else {
          text = input_p2_name.value + " (Player 2)";
        }
      } else {
        if (input_p1_name.value == "" || input_p1_name.value == "Player 2") {
          text = "Player 2";
        } else {
          text = input_p1_name.value + " (Player 2)";
        }
      }
    } else {
      text = audioName;
    }
    popup_text.innerHTML = text;
  }

  async function record_output_audio() {
    if (recording_output)
      return;
    recording_output = true;
    channel.length = 0;
    recordAudioCounter = 0;
    const stream = await navigator.mediaDevices.getUserMedia({ audio: true });
    popup.style.display = "flex";
    popup_header.innerHTML = "Say the following word:";
    setAudioName();
    getUserMedia(stream);

  };


  var channel = [];
  var source;
  getUserMedia = (stream) => {
    audioContextType = window.AudioContext || window.webkitAudioContext;
    track = stream.getTracks()[0];
    // create the MediaStreamAudioSourceNode
    // Setup Audio Context
    context = new audioContextType();
    source = context.createMediaStreamSource(stream);

    // create a ScriptProcessorNode
    if (!context.createScriptProcessor) {
      node = context.createJavaScriptNode(Recognize.bufferSize, numChannels, numChannels);
    } else {
      node = context.createScriptProcessor(Recognize.bufferSize, numChannels, numChannels);
    }

    // listen to the audio data, and record into the buffer, this is important to catch the fraction of second before the speech started.
    node.onaudioprocess = (e) => {
      if (!recording_output) {
        node.disconnect();
        return;
      }
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
    source.connect(node);
    node.connect(context.destination);

    var speechEvents = hark(stream, { interval: _harkInterval2, threshold: _threshold2, play: false, recoredInterval: _stopRecTimeout2 });

    speechEvents.on('speaking', function () {
      if(!recording_output){
        speechEvents.stop();
        return;
      }
      speaking = true;
      console.log('speaking');
      popup_container.style.background = "Green";
    });

    speechEvents.on('stopped_speaking', function () {
      if(!recording_output){
        speechEvents.stop();
        return;
      }
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



      const audioUrl = URL.createObjectURL(Utils.bufferToBlob(channel, recordingLength));
      const audio = new Audio(audioUrl);

      recordedAudio[audioName] = audio;


      channel.length = 0;
      recordingLength = 0;

      recordAudioCounter++;

      console.log(recordAudioCounter + "-" + requiredAudio.length);
      if (recordAudioCounter >= requiredAudio.length) {
        track.stop();
        speechEvents.stop();
        recording_output = false;
        popup.style.display = "none";
        node.disconnect();
      } else {
        setAudioName();
      }
    });
  };
  window.addEventListener('load', (event) => {
    record_output_audio_button.onclick = record_output_audio;
});
})()