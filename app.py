import streamlit as st
import tempfile
import os
import subprocess

# Function to run `./shazam` with an audio file
def find_song(audio_path):
    result = subprocess.run(["./shazam", audio_path], capture_output=True, text=True)
    return result.returncode, result.stdout  

# Function to run `./add` with song details
def add_song(file_path, song_name, artist_name):
    result = subprocess.run(["./add", file_path, song_name, artist_name], capture_output=True, text=True)
    return result.returncode, result.stdout 

# Streamlit UI
st.title("🎵 SeekTune")
st.subheader("Find songs by uploading an audio file or adding new songs to the database.")

# Create Tabs: Upload & Match | Add New Song
tab1, tab2 = st.tabs(["Upload & Match", "Add New Song"])

# 🎤 Tab 1: Upload Audio & Match
with tab1:
    st.subheader("Upload & Match a Song")
    uploaded_file = st.file_uploader("Upload an MP3 file", type=["mp3"])
    
    if uploaded_file is not None:
        with tempfile.NamedTemporaryFile(delete=False, suffix=".mp3") as temp_mp3:
            temp_mp3.write(uploaded_file.read())
            temp_mp3_path = temp_mp3.name

        st.success("File uploaded successfully!")
        st.audio(temp_mp3_path, format='audio/mp3')

        if st.button("🔍 Match Song"):
            with st.spinner("Matching song..."):
                return_code, output = find_song(temp_mp3_path)
                if return_code == 0:
                    st.success("Match Found!")
                else:
                    st.error("No match found or an error occurred.")
                st.write("**Match Result:**", output)
                os.remove(temp_mp3_path)  # Cleanup temp file

# 🎵 Tab 2: Add a New Song to Database
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
                os.remove(temp_song_path)  # Cleanup temp file
