#ifndef EXTERN_PLAYER_EVENT_H
#define EXTERN_PLAYER_EVENT_H

#define PLAYER_EVENT_UPGRADE(p, x) do { \
 if ((p)->event < PLAYER_EVENT_ ## x) \
   (p)->event = PLAYER_EVENT_ ## x; \
 } while (FALSE);

#define PLAYER_EVENT_SET_FROM(p, x, y) do { \
 if ((p)->event == PLAYER_EVENT_ ## x) \
   (p)->event = PLAYER_EVENT_ ## y; \
 } while (FALSE);

extern int player_event_do(player_linked_list *, va_list);

#endif
