import streamlit as st
import tempfile
import os
import subprocess
import sounddevice as sd
import scipy.io.wavfile as wav
import pandas as pd
from pymongo import MongoClient


def fetch_songs():
    try:
        client = MongoClient("mongodb://localhost:27017")  
        db = client["song-recognition"]
        collection = db["songs"]

        songs = list(collection.find({}))  
        
        song_list = []
        for song in songs:
            song_id = song.get("_id", "N/A")
            key = song.get("key", "Unknown")
            title, artist = key.split("---") if "---" in key else (key, "Unknown")

            song_list.append({"ID": song_id, "Title": title, "Artist": artist})

        return song_list
    except Exception as e:
        st.error(f"Error fetching songs: {e}")
        return []


def record_audio(duration=13, sample_rate=44100):
     audio_data = sd.rec(int(duration * sample_rate), samplerate=sample_rate, channels=1, dtype='int16')
     sd.wait()
     temp_wav = tempfile.NamedTemporaryFile(delete=False, suffix=".wav")
     wav.write(temp_wav.name, sample_rate, audio_data)
     temp_mp3 = temp_wav.name.replace(".wav", ".mp3")
     subprocess.run(["ffmpeg", "-i", temp_wav.name, "-q:a", "2", temp_mp3], stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
 
     os.remove(temp_wav.name) 
     return temp_mp3
     

def find_song(audio_path):
    result = subprocess.run(["build/shazam", audio_path], capture_output=True, text=True)
    return result.returncode, result.stdout  


def add_song(file_path, song_name, artist_name):
    result = subprocess.run(["build/add", file_path, song_name, artist_name], capture_output=True, text=True)
    return result.returncode, result.stdout 


st.title("Look-It-Up")


tab1, tab2, tab3 = st.tabs(["Search", "Add New Song to Database","View Database"])

with tab1:
    st.subheader("Search a Song")
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
    st.write("")
    st.write("")
    col1, col2, col3 = st.columns([1, 2, 1])
    with col2:
        if st.button("Start Recording", use_container_width=True):
            with st.spinner("Recording... Speak now!"):
                audio_path = record_audio()


            with st.spinner("Matching song..."):
                return_code, output = find_song(audio_path)

            if return_code == 0:
                st.success("Match Found!")
                st.write("**Match Result:**\n", output)
            else:
                st.error("No match found or an error occurred.")

            os.remove(audio_path)       
     

                



with tab2:
    st.subheader("Add a New Song")
    song_file = st.file_uploader("Upload an MP3 file to add", type=["mp3"], key="add_song")
    song_name = st.text_input("Enter Song Name")
    artist_name = st.text_input("Enter Artist Name")

    if song_file and song_name and artist_name:
        with tempfile.NamedTemporaryFile(delete=False, suffix=".mp3") as temp_song:
            temp_song.write(song_file.read())
            temp_song_path = temp_song.name

        if st.button("Submit"):
            with st.spinner("Adding song to database..."):
                return_code, output = add_song(temp_song_path, song_name, artist_name)
                if return_code == 0:
                    st.success("Song added successfully!")
                else:
                    st.error("Failed to add song.")
                st.text(output)
                os.remove(temp_song_path)  


with tab3:
    st.subheader("Songs in Database")
    with st.spinner("Fetching songs..."):
            song_data = fetch_songs()

            if song_data:
                df = pd.DataFrame(song_data)
                st.dataframe(df.set_index("ID"), use_container_width=True)
            else:
                st.info("No songs found in the database.")
        
