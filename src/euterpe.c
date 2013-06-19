#include "euterpe.h"

int debug = 0;
audio_fifo_t g_audio;

static int g_logged_in = 0;
static int g_track_num = 0;
static sp_playlist *g_playlist = NULL;
static pthread_t g_thread;
static pthread_cond_t g_action_needed;
static pthread_mutex_t g_mutex;

int euterpe_init(sp_session** sess, const char* username, 
                   const char* password, const char* blob) {

  sp_session_config sess_conf;
  sp_session_callbacks callbacks;
  sp_error err;
  
  extern const char g_appkey[];
  extern const size_t g_appkey_size;

  /* Error checking: */
  if (username == NULL)  {
    fprintf(stderr, "Unable to login: No username\n");
    return 1;
  }
  if (password == NULL) {
    fprintf(stderr, "Unable to login: No password\n");
    return 1;
  }

  /* Set pre-login configuration: */
  callbacks.logged_in = &logged_in;
  callbacks.logged_out = &euterpe_exit;
  callbacks.metadata_updated = &metadata_updated;
  callbacks.connection_error = &connection_error;
  callbacks.message_to_user = &message_to_user;
  callbacks.notify_main_thread = &euterpe_loop_activate;
  callbacks.music_delivery = &music_delivery;
  callbacks.play_token_lost = &play_token_lost;
  callbacks.log_message = &log_message;
  callbacks.end_of_track = &end_of_track;
  callbacks.streaming_error = &streaming_error;
  callbacks.userinfo_updated = &userinfo_updated;
  callbacks.start_playback = &start_playback;
  callbacks.stop_playback = &stop_playback;
  callbacks.get_audio_buffer_stats = &get_audio_buffer_stats;
  callbacks.offline_status_updated = &offline_status_updated;
  callbacks.offline_error = &offline_error;
  callbacks.credentials_blob_updated = &credentials_blob_updated;
  callbacks.connectionstate_updated = &connectionstate_updated;
  callbacks.scrobble_error = &scrobble_error;
  callbacks.private_session_mode_changed = &private_session_mode_changed;

  memset(&sess_conf, 0, sizeof(sess_conf));
  sess_conf.api_version = SPOTIFY_API_VERSION;
  sess_conf.cache_location = ".cache";
  sess_conf.settings_location = ".cache";
  sess_conf.application_key = g_appkey;
  sess_conf.application_key_size = g_appkey_size;
  sess_conf.user_agent = "euterpe";
  sess_conf.compress_playlists = false;
  sess_conf.callbacks = &callbacks;
  
  /* Try to init session: */
  audio_init(&g_audio);
  err = sp_session_create(&sess_conf, sess);
  if (err != SP_ERROR_OK) {
    fprintf(stderr, "Failed to create session: %s\n", sp_error_message(err));
    return 1;
  }
  
  /* Blob <-- Maybe remove??
  if (blob != NULL) {
    if ((err = sp_session_relogin(*sess)) == SP_ERROR_OK) {
      fprintf(stdout, "Sent blob relogin request\n");
      g_logged_in = 1;
      return 0;
    }
    fprintf(stderr, "Unable to blob relogin: %s\n", sp_error_message(err));
    fprintf(stdout, "Attempting normal login\n");
  } */

  g_logged_in = 1;
  sp_session_login(*sess, username, password, 0, blob);
  fprintf(stdout, "Attempting login..\n");

  pthread_cond_init(&g_action_needed, NULL);
  pthread_create(&g_thread, NULL, euterpe_loop, sess);

  return 0;
}

void euterpe_exit(sp_session* sess) {

  fprintf(stdout, "Logging out..\n");
  
  g_logged_in = 0;
  sp_session_logout(sess);
  pthread_cond_signal(&g_action_needed);
  pthread_join(g_thread, NULL);
  pthread_cond_destroy(&g_action_needed);

  exit(0);

}

void *euterpe_loop(void* arg) {

  sp_session *sess = *(sp_session **)arg;
  int sleep_time = 0;
  struct timespec ts;
  memset(&ts, 0, sizeof(ts));

  while (g_logged_in) {

    /* Sleep if needed */
    if (sleep_time != 0)
      pthread_cond_timedwait(&g_action_needed, &g_mutex, &ts);
    
    do {
      sp_session_process_events(sess, &sleep_time);
    } while (sleep_time == 0);
    
    clock_gettime(CLOCK_REALTIME, &ts);
    ts.tv_sec += sleep_time / 1000;
    ts.tv_nsec += (sleep_time % 1000) * 1000000;
    if (ts.tv_nsec >= 1000000000) {
      ts.tv_sec += 1;
      ts.tv_nsec -= 1000000000;
    }
    if (debug)
      printf("\033[1;30m%d until next event\n\033[0m", sleep_time/1000);
  }
  return NULL;
}

void euterpe_loop_activate(sp_session* sess) {
  UNUSED(sess);
  if (debug)
    fprintf(stdout, "\033[1;30mnotify main thread!\033[0m\n");
  pthread_cond_signal(&g_action_needed);
}

void euterpe_play_list(sp_session* sess, int listnum) {
  
   sp_playlistcontainer *cont = sp_session_playlistcontainer(sess);
   int num_lists;
   
   if (cont == NULL) {
     printf("Could not find any list since you're not logged in\n");
     return;
   }
   
   num_lists = sp_playlistcontainer_num_playlists(cont);
   
   if (-1 > listnum || listnum >= num_lists) {
     printf("Could not find playlist %d", listnum);
     return;
   }
   
   g_track_num = 0;
   g_playlist = sp_playlistcontainer_playlist(cont, listnum);
   euterpe_change_track(sess, 0, 0);

}

void euterpe_change_track(sp_session* sess, int force_set_track, int modifier) {
  
  int available_tracks; 
  sp_track *track = NULL;
  sp_error err;
  int availerr;
  
  if (g_playlist == NULL) 
    return;
  
  available_tracks = sp_playlist_num_tracks(g_playlist);

  if (force_set_track) {
    if (0 > modifier || modifier > available_tracks -1) {
      printf("Please set a number between 0-%d\n", available_tracks -1);
      return;
    }
    g_track_num = modifier;
  }

  else {
    g_track_num += modifier;

    while (g_track_num >= available_tracks)
      g_track_num -= available_tracks;
    while (g_track_num < 0)
      g_track_num += available_tracks;
  }



  audio_fifo_flush(&g_audio);
  sp_session_player_unload(sess);
  
  track = sp_playlist_track(g_playlist, g_track_num);
  printf("%d: %s - %s\n", g_track_num, sp_artist_name(sp_track_artist(track, 0)),
         sp_track_name(track));
  
  sp_session_player_load(sess, sp_playlist_track(g_playlist, g_track_num));
  sp_session_player_play(sess, 1);

  if ((err = sp_track_error(track)) != SP_ERROR_OK)
    printf("Unable to play track: %s\n", sp_error_message(err));
  
  else if ((availerr = sp_track_get_availability(sess, track)) 
           != SP_TRACK_AVAILABILITY_AVAILABLE) {
    printf("Unable to play track: ");
    switch(availerr) {
      case 0: printf("Track is not available\n"); break;
      case 2: printf("Track is not streamable on this account\n"); break;
      case 3: printf("Track is banned by artist's request\n"); break;
      default: printf("Unknown availability error\n"); break;
    }
    printf("Skipping to next ..\n");
    euterpe_change_track(sess, 0, 1);
  }
}

void euterpe_display_playlists(sp_session* sess) {

  sp_playlistcontainer *cont = sp_session_playlistcontainer(sess);
  int i, num_lists;
  
  if (cont == NULL) {
    printf("Could not find any list since you're not logged in\n");
    return;
  }

  num_lists = sp_playlistcontainer_num_playlists(cont);
    
  printf("--- %d Playlists: ---\n", num_lists);
  for (i = 0; i < num_lists; i++) {
    sp_playlist *list = sp_playlistcontainer_playlist(cont, i);
    printf("%d: %s\n", i, sp_playlist_name(list));
  }
}

void euterpe_display_tracks(sp_session *sess, int playlist_num) {

  sp_playlistcontainer *cont = sp_session_playlistcontainer(sess);
  int i, num_tracks, num_lists;
  sp_track *track = NULL;
  sp_playlist *playlist = NULL;

  /* Display current playlist: */
  if (playlist_num == -1) {

    if (g_playlist == NULL) {
      printf("No playlist selected\n");
      return;
    }
    
    else
      playlist = g_playlist;
  }
  
  /* Display playlist <playlist_num>: */
  else {

    num_lists = sp_playlistcontainer_num_playlists(cont);
 
    if (0 > playlist_num || playlist_num >= num_lists) {
      printf("Could not find playlist\n");
      return;
    }

    else
      playlist = sp_playlistcontainer_playlist(cont, playlist_num);
  }

  num_tracks = sp_playlist_num_tracks(playlist);

    printf("--- %s has %d tracks: ---\n", sp_playlist_name(playlist), num_tracks);
    for (i = 0; i < num_tracks; i++) {
      track = sp_playlist_track(playlist, i);
      printf("%d: %s - %s\n", i, sp_artist_name(sp_track_artist(track, 0)), 
             sp_track_name(track));
    }
}
