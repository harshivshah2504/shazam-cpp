import subprocess
import time
from pydub import AudioSegment
import pyaudio
import wave

# Function to record audio
def record_audio(duration, sample_rate=44100, channels=1, chunk_size=1024):
    p = pyaudio.PyAudio()

    # Open the stream
    stream = p.open(format=pyaudio.paInt16,
                    channels=channels,
                    rate=sample_rate,
                    input=True,
                    frames_per_buffer=chunk_size)

    print("Recording...")
    frames = []
    for _ in range(0, int(sample_rate / chunk_size * duration)):
        data = stream.read(chunk_size)
        frames.append(data)

    print("Recording finished.")
    stream.stop_stream()
    stream.close()
    p.terminate()

    # Save the audio as a .wav file
    with wave.open("recorded_audio.wav", "wb") as wf:
        wf.setnchannels(channels)
        wf.setsampwidth(p.get_sample_size(pyaudio.paInt16))
        wf.setframerate(sample_rate)
        wf.writeframes(b''.join(frames))

# Function to replay the audio
def replay_audio():
    audio = AudioSegment.from_wav("recorded_audio.wav")
    print("Replaying audio...")
    # audio.export("replayed_audio.mp3", format="mp3")  
    time.sleep(audio.duration_seconds)  

# Function to invoke the C++ function from Python
def process_audio_with_cpp():
    # Assuming your C++ program is called "audio_processor" and takes a file path as an argument
    subprocess.run(["./shazam", "replayed_audio.mp3"], check=True)

# Main function
def main():
    duration = float(input("Enter duration in seconds: "))
    record_audio(duration)
    
    replay_audio()
    
    # Pass the saved .mp3 file to the C++ program for processing
    process_audio_with_cpp()

if __name__ == "__main__":
    main()
