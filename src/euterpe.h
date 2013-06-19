#ifndef EUTERPE_H_
#define EUTERPE_H_

#define UNUSED(x) (void)(x)

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h>
#include <unistd.h>
#include <pthread.h>

#include <libspotify/api.h>
#include "alsa_audio.h"

/* From spotify_callsbacks.c: */
void logged_in(sp_session* sess, sp_error err);
void metadata_updated(sp_session* sess);
void connection_error(sp_session* sess, sp_error err);
void message_to_user(sp_session* sess, const char* message);
int music_delivery(sp_session* sess, const sp_audioformat* format, 
                   const void* frames, int num_frames);
void play_token_lost(sp_session* sess);
void log_message(sp_session* sess, const char* data);
void end_of_track(sp_session* sess);
void streaming_error(sp_session* sess, sp_error err);
void userinfo_updated(sp_session* sess);
void start_playback(sp_session* sess);
void stop_playback(sp_session* sess);
void get_audio_buffer_stats(sp_session* sess, sp_audio_buffer_stats *stats);
void offline_status_updated(sp_session* sess);
void offline_error(sp_session* sess, sp_error err);
void credentials_blob_updated(sp_session* sess, const char* blob);
void connectionstate_updated(sp_session* sess);
void scrobble_error(sp_session* sess, sp_error err);
void private_session_mode_changed(sp_session* sess, bool is_private);

/* From euterpe.c: */
int euterpe_init(sp_session** sess, const char* username, 
                   const char* password, const char* blob);
void euterpe_exit(sp_session* sess);
void *euterpe_loop(void* arg);
void euterpe_loop_activate(sp_session* sess);
void euterpe_play_list(sp_session* sess, int listnum);
void euterpe_change_track(sp_session* sess, int force_set_track, int modifier);
void euterpe_display_playlists(sp_session* sess);
void euterpe_display_tracks(void);

#endif
