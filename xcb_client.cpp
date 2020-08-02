#include <iostream>

#include <xcb/xcb.h>

int main() {
  int screen_nbr = 0;
  /* Open the connection to the X server. Use the DISPLAY environment variable as the default display name */
  xcb_connection_t *c = xcb_connect(NULL, &screen_nbr);
  if (xcb_connection_has_error(c)) {
    auto env_dp = std::getenv("DISPLAY");
    if (env_dp) {
      std::cerr << "Cannot open display: '" << env_dp << "'" << std::endl;
    } else {
      std::cerr << "DISPLAY environment variable not set " << std::endl;
    }
    return 1;
  }
  auto screen = xcb_setup_roots_iterator(xcb_get_setup(c)).data;

  // get the first screen
//  xcb_screen_t *screen = nullptr;
//  for (; iter.rem; --screen_nbr, xcb_screen_next(&iter)) {
//    if (screen_nbr == 0) {
//      screen = iter.data;
//      break;
//    }
//  }
  if (!screen) {
    xcb_disconnect(c);
    std::cerr << "Failed to get first screen " << std::endl;
    return 1;
  }

  std::cout << "Root: " << screen->root << " "
            << screen->width_in_pixels << "x" << screen->height_in_pixels << "px, "
            << screen->width_in_millimeters << "x" << screen->height_in_millimeters<< "mm"
            << std::endl;

  uint32_t values[2];
  auto foreground = xcb_generate_id (c);
  auto mask = XCB_GC_FOREGROUND | XCB_GC_GRAPHICS_EXPOSURES;
  values[0] = screen->black_pixel;
  values[1] = 0;
  auto cookie = xcb_create_gc_checked (c, foreground, screen->root, mask, values);
  auto err = xcb_request_check(c, cookie);
  if (err) {
    std::cerr << "Failed to create gc " << err->error_code << std::endl;
    return 1;
  }

  /* Ask for our window's Id */
  auto win = xcb_generate_id(c);

  mask = XCB_CW_BACK_PIXEL | XCB_CW_EVENT_MASK;
  values[0] = screen->white_pixel;
  values[1] = XCB_EVENT_MASK_EXPOSURE;
  xcb_create_window (c,                             /* Connection          */
                     XCB_COPY_FROM_PARENT,          /* depth (same as root)*/
                     win,                           /* window Id           */
                     screen->root,                  /* parent window       */
                     0, 0,                          /* x, y                */
                     150, 150,                      /* width, height       */
                     10,                            /* border_width        */
                     XCB_WINDOW_CLASS_INPUT_OUTPUT, /* class               */
                     screen->root_visual,           /* visual              */
                     mask, values);                      /* masks, not used yet */

  /* Map the window on the screen */
  xcb_map_window (c, win);
  /* Make sure commands are sent before we pause, so window is shown */
  xcb_flush (c);

  /* geometric objects */
  xcb_point_t          points[] = {
      {10, 10},
      {10, 20},
      {20, 10},
      {20, 20}};

  xcb_point_t          polyline[] = {
      {50, 10},
      { 5, 20},     /* rest of points are relative */
      {25,-20},
      {10, 10}};

  xcb_segment_t        segments[] = {
      {100, 10, 140, 30},
      {110, 25, 130, 60}};

  xcb_rectangle_t      rectangles[] = {
      { 10, 50, 40, 20},
      { 80, 50, 10, 40}};

  xcb_arc_t            arcs[] = {
      {10, 100, 60, 40, 0, 90 << 6},
      {90, 100, 55, 40, 0, 270 << 6}};


  xcb_generic_event_t *e = nullptr;
  while ((e = xcb_wait_for_event (c))) {
    switch (e->response_type & ~0x80) {
      case XCB_EXPOSE: {
        /* We draw the points */
        xcb_poly_point (c, XCB_COORD_MODE_ORIGIN, win, foreground, 4, points);

        /* We draw the polygonal line */
        xcb_poly_line (c, XCB_COORD_MODE_PREVIOUS, win, foreground, 4, polyline);

        /* We draw the segements */
        xcb_poly_segment (c, win, foreground, 2, segments);

        /* We draw the rectangles */
        xcb_poly_rectangle (c, win, foreground, 2, rectangles);

        /* We draw the arcs */
        xcb_poly_arc (c, win, foreground, 2, arcs);

        /* We flush the request */
        xcb_flush (c);

        break;
      }
      default: {
        /* Unknown event type, ignore it */
        break;
      }
    }
    /* Free the Generic Event */
    free (e);
  }



  int a;
  std::cin >> a;


  xcb_disconnect(c);
}
