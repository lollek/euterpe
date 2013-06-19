TARGET=euterpe
CFLAGS=-Wall -Wextra -pedantic
LDFLAGS=-lspotify -lasound

OBJS += src/main.o src/euterpe.o src/spotify_callbacks.o src/appkey.o src/alsa_audio.o

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) $^ $(LDFLAGS) -o $@

.PHONY: clean test
test: $(TARGET)
	./$(TARGET)

clean:
	$(RM) src/*.o $(TARGET)
