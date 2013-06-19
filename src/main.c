#include "euterpe.h"

extern int debug;

void handle_input(sp_session *sess) {

  char cmd[256], *p;
  
  for (;;) {
    
    printf("\n> ");
    fgets(cmd, sizeof(cmd), stdin);
    p = cmd;
    
    switch(*p) {

      case 'd': {
        printf("Debugmode changed to %s\n", debug? "off":"on");
        debug = !debug;
      } break;

      case 'h': {
        printf("Commands:\n\n"

               "next\t\t\t - next track\n"
               "prev\t\t\t - previous track\n\n"

               "d\t\t\t - toggle debug mode\n"
               "l\t\n"
               " t / tracks\t\t - list tracks\n"
               " p / playlists\t\t - list playlists\n"
               "p\t\n"
               " l <number>\t\t - play list\n"
               " t <number>\t\t - play track\n"
               "q\t\t\t - quit\n");
      } break;
        
      case 'l': {
        if (strlen(cmd) > 2 && !strncmp(cmd, "l ", 2)) {
          if (!strcmp(cmd, "l t\n"))
            euterpe_display_tracks();
          else if (strlen(cmd) >= 7 && !strncmp(cmd, "l track\n", 7))
            euterpe_display_tracks();
          else if (!strcmp(cmd, "l p\n"))
            euterpe_display_playlists(sess);
          else if (strlen(cmd) >= 10 && !strcmp(cmd, "l playlist\n")) 
            euterpe_display_playlists(sess);
          else
            printf("Unknown command. Type h for help\n");
        }
      } break;

      case 'n': {
        if (!strcmp(cmd, "next\n"))
          euterpe_change_track(sess, 0, 1);
      } break;
      
      case 'p': {
        if (!strcmp(cmd, "prev\n"))
          euterpe_change_track(sess, 0, -1);
        else if (strlen(cmd) > 4 && !strncmp(cmd, "p t ", 4))
          euterpe_change_track(sess, 1, atoi(cmd + 4));
        else if (strlen(cmd) > 4 && !strncmp(cmd, "p l ", 4))
          euterpe_play_list(sess, atoi(cmd + 4));
        else
          printf("Unknown command. Type h for help\n");
      } break;

      case 'q': { 
        euterpe_exit(sess); 
      } break;

      default: 
        printf("Unknown command. Type h for help\n");
        break;
    }
  }
}

int main(int argc, char** argv) {

  char c, buf[256];
  char *username = NULL;
  char *password = NULL;
  char *blob = NULL;
  sp_session *sess = NULL;
  
  fprintf(stdout, "Euterpe v0.1\n\n");
  
  while ((c = getopt(argc, argv, "dl:u:")) != -1) {
    switch (c) {
      case 'd': debug = 1; break;
      case 'u': username = optarg; break;
      default: break;
    }
  }
  
  if (username == NULL) {
    printf("Username: ");
    fflush(stdout);
    fgets(buf, sizeof(buf), stdin);
    username = buf;
  }

  if (strlen(username) < 2) {
    printf("Blank username received\n");
    return 1;
  }
  
  password = getpass("Password: ");
  if (strlen(password) < 1) {
    printf("Blank password received\n");
    return 1;
  }
  
  if (euterpe_init(&sess, username, password, blob) != 0
      || sess == NULL) {
    printf("Failed to create a spotify session\n");
    return 1;
  }
  free(password);
  
  handle_input(sess);
  
  euterpe_exit(sess);

  return 0;
}
