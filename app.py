import streamlit as st
import numpy as np
import sounddevice as sd
import tempfile
import os
import subprocess
import scipy.io.wavfile as wav

# Function to record and save audio as MP3
def record_audio(duration=8, sample_rate=44100):
    st.info("Recording... Speak now!")
    
    audio_data = sd.rec(int(duration * sample_rate), samplerate=sample_rate, channels=1, dtype='int16')
    sd.wait()

    # Save to temporary WAV file
    temp_wav = tempfile.NamedTemporaryFile(delete=False, suffix=".wav")
    wav.write(temp_wav.name, sample_rate, audio_data)

    # Convert WAV to MP3 using ffmpeg
    temp_mp3 = temp_wav.name.replace(".wav", ".mp3")
    subprocess.run(["ffmpeg", "-i", temp_wav.name, "-q:a", "2", temp_mp3], stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)

    os.remove(temp_wav.name)  # Delete temporary WAV file
    return temp_mp3

# Function to run `./shazam` with an audio file
def find_song(audio_path):
    result = subprocess.run(["./shazam", audio_path], capture_output=True, text=True)
    return result.stdout  # Return output from `./shazam`

# Function to run `./add` with song details
def add_song(file_path, song_name, artist_name):
    result = subprocess.run(["./add", file_path, song_name, artist_name], capture_output=True, text=True)
    return result.stdout  # Return output from `./add`

# Streamlit UI
st.title("🎵 SeekTune")
st.subheader("Find songs by recording or uploading an audio file.")

# # Display current song count (Static for now, can be dynamic)
# st.write("**35 Songs In Database**")



# Create Tabs: Record & Upload
tab1, tab2 = st.tabs(["🎤 Record Audio", "Add New Song"])

# 🎤 Tab 1: Record Audio & Match
with tab1:
    st.subheader("Record & Match a Song")
    # Listening Animation
    st.markdown("""
        <style>
        @keyframes pulse {
            0% { box-shadow: 0 0 10px #1E90FF; }
            50% { box-shadow: 0 0 40px #1E90FF; }
            100% { box-shadow: 0 0 10px #1E90FF; }
        }
        .listening {
            width: 100px;
            height: 100px;
            background-color: #1E90FF;
            border-radius: 50%;
            display: flex;
            align-items: center;
            justify-content: center;
            color: white;
            font-weight: bold;
            animation: pulse 1.5s infinite;
            margin: auto;
        }
        </style>
        <div class='listening'>Listening...</div>
    """, unsafe_allow_html=True)

    
    if st.button("🎤 Start Recording"):
        audio_path = record_audio()
        st.success(f"Recording saved as {audio_path}. Running fingerprint match...")
        output = find_song(audio_path)
        st.write("🎶 **Match Result:**", output)
        os.remove(audio_path)  # Cleanup temp file

# 🎵 Tab 2: Add a New Song to Database
with tab2:
    st.subheader("Add a New Song")
    song_path = st.text_input("Enter full local file path (MP3/WAV)")
    song_name = st.text_input("Enter Song Name")
    artist_name = st.text_input("Enter Artist Name")

    if st.button("Submit"):
        if song_path and song_name and artist_name:
            output = add_song(song_path, song_name, artist_name)
            st.success("Song added successfully!")
            st.text(output)
        else:
            st.error("Please provide all song details.")
