#include "board_display.hpp"

#include <gtk/gtk.h>
#include <gdk/gdkkeysyms.h>
#include <librsvg/rsvg.h>

#include <cmath>
#include <cstdlib>
#include <cstring>

/**** Drawing with cairo and librsvg ****/

static void draw_id(
    cairo_t*    cr,
    const RsvgHandle* style,
    const char* id,
    double      unit,
    double      scale,
    double      line,
    double      column,
    double      angle,
    int         mirror
    )
{
  /* save the transformation matrix */
  cairo_save(cr) ;

  /* move to the draw location (with center correction) */
  cairo_translate(cr, (column+0.5)*unit, (line+0.5)*unit) ;

  /* apply rotation and mirror if any */
  if(mirror) cairo_scale(cr, -1, 1) ;
  cairo_rotate(cr, angle) ;

  /* undo center correction */
  cairo_translate(cr, -0.5*unit, -0.5*unit) ;

  /* apply scale */
  cairo_scale(cr, scale, scale) ;

  /* draw */
  rsvg_handle_render_cairo_sub((RsvgHandle*) style, cr, id) ;

  /* restore the transformation matrix */
  cairo_restore(cr) ;
}

/**** Tiles ****/

static void draw_tile_background(
    cairo_t* cr,
    const RsvgHandle* style,
    double unit,
    double scale,
    const RRTile* tile
    )
{
  if(tile->type != RR_TILE_NONE)
  {
    draw_id(
        cr, 
        style, 
        "#background", 
        unit, 
        scale, 
        tile->line, 
        tile->column, 
        0,
        0
        ) ;
  }
}

static void draw_tile_type(
    cairo_t* cr,
    const RsvgHandle* style,
    double unit,
    double scale,
    const RRTile* tile
    )
{
  if(tile->type == RR_TILE_NOTHING || tile->type == RR_TILE_NONE)
  {
    /* nothing to draw, empty tile */
    return ;
  }
  else if(tile->type < RR_TILE_TURN_SE) 
  {
    /* belt, types ordered to compute angle */
    draw_id(
        cr, 
        style, 
        "#forward", 
        unit, 
        scale, 
        tile->line, 
        tile->column, 
        -M_PI/2*(tile->type - RR_TILE_BELT_E),
        0
        ) ;
  } 
  else if(tile->type < RR_TILE_JOIN_S_TO_E) 
  {
    /* belt turn, types ordered to compute angle and mirror*/
    draw_id(
        cr, 
        style, 
        "#turn", 
        unit, 
        scale, 
        tile->line, 
        tile->column, 
        -M_PI/2*((tile->type - RR_TILE_TURN_SE)/2),
        (tile->type - RR_TILE_TURN_SE) % 2
        ) ;
  } 
  else if(tile->type < RR_TILE_JOIN_NS_TO_E) 
  {
    /* perpendicular join tile, types ordered to compute angle and mirror */
    draw_id(
        cr, 
        style, 
        "#perp_join", 
        unit, 
        scale, 
        tile->line, 
        tile->column, 
        -M_PI/2*((tile->type - RR_TILE_JOIN_S_TO_E)/2),
        (tile->type - RR_TILE_JOIN_S_TO_E)%2
        ) ;
  } 
  else if(tile->type < RR_TILE_FAST_BELT_E) 
  {
    /* opposite join tile, types ordered to compute angle and mirror */
    draw_id(
        cr, 
        style, 
        "#opp_join", 
        unit, 
        scale, 
        tile->line, 
        tile->column, 
        -M_PI/2*(tile->type - RR_TILE_JOIN_NS_TO_E),
        0
        ) ;
  } 
  else if(tile->type < RR_TILE_FAST_TURN_SE)
  {
    /* fast belt, types ordered to compute angle */
    draw_id(
        cr, 
        style, 
        "#fast_forward", 
        unit, 
        scale, 
        tile->line, 
        tile->column, 
        -M_PI/2*(tile->type - RR_TILE_FAST_BELT_E),
        0
        ) ;
  }
  else if(tile->type < RR_TILE_FAST_JOIN_S_TO_E) 
  {
    /* fast_belt_turn, types ordered to compute angle and mirror*/
    draw_id(
        cr, 
        style, 
        "#fast_turn", 
        unit, 
        scale, 
        tile->line, 
        tile->column, 
        -M_PI/2*((tile->type - RR_TILE_FAST_TURN_SE)/2),
        (tile->type - RR_TILE_FAST_TURN_SE) % 2
        ) ;
  } 
  else if(tile->type < RR_TILE_FAST_JOIN_NS_TO_E) 
  {
    /* perpendicular join tile, types ordered to compute angle and mirror */
    draw_id(
        cr, 
        style, 
        "#fast_perp_join", 
        unit, 
        scale, 
        tile->line, 
        tile->column, 
        -M_PI/2*((tile->type - RR_TILE_FAST_JOIN_S_TO_E)/2),
        (tile->type - RR_TILE_FAST_JOIN_S_TO_E)%2
        ) ;
  } 
  else if(tile->type < RR_TILE_ROTATE_RIGHT) 
  {
    /* opposite join tile, types ordered to compute angle and mirror */
    draw_id(
        cr, 
        style, 
        "#fast_opp_join", 
        unit, 
        scale, 
        tile->line, 
        tile->column, 
        -M_PI/2*(tile->type - RR_TILE_JOIN_NS_TO_E),
        0
        ) ;
  } 
  else
  {
    /* rotating tile, types ordered to compute mirror */
    draw_id(
        cr, 
        style, 
        "#rotate", 
        unit, 
        scale, 
        tile->line, 
        tile->column, 
        0,
        tile->type - RR_TILE_ROTATE_RIGHT
        ) ;
  }
}

/* generic comparison for tiles */

static int tile_compare(
    const void* generic_tile_1,
    const void* generic_tile_2
    )
{
  RRTile* tile_1 = (RRTile*) generic_tile_1 ;
  RRTile* tile_2 = (RRTile*) generic_tile_2 ;

  int c = (tile_1->line > tile_2->line) - (tile_1->line < tile_2->line) ;
  if(c) return c ;

  return (tile_1->column > tile_2->column) - (tile_1->column < tile_2->column) ;
}

/* compare function to sort tiles, with none tiles in the end */

static int tile_compare_none(
    const void* generic_tile_1,
    const void* generic_tile_2
    )
{
  RRTile* tile_1 = (RRTile*) generic_tile_1 ;
  RRTile* tile_2 = (RRTile*) generic_tile_2 ;

  if(tile_1->type == RR_TILE_NONE && ! (tile_2->type == RR_TILE_NONE))
    return 1 ;

  if(tile_2->type == RR_TILE_NONE && ! (tile_1->type == RR_TILE_NONE))
    return -1 ;

  return tile_compare(generic_tile_1, generic_tile_2) ;
}

/* search in a sorted tile array */

static RRTile* tile_search (
    const RRTile* tile_start,
    unsigned int tile_size,
    const RRTile* query
    )
{
  return (RRTile*) bsearch(
      query,
      tile_start,
      tile_size,
      sizeof(RRTile),
      tile_compare
      ) ;
}

/**** Robot ****/

static void draw_robot (
    cairo_t* cr,
    const RsvgHandle* style,
    double unit,
    double scale,
    const RRRobot* robot
    )
{
  draw_id(
      cr, 
      style, 
      "#robot", 
      unit, 
      scale, 
      robot->line, 
      robot->column, 
      -M_PI/2*robot->status,
      0
      ) ;
  /* TODO handle dead robot */
}

static void draw_previous_robot (
    cairo_t* cr,
    const RsvgHandle* style,
    double unit,
    double scale,
    const RRRobot* robot
    )
{
  draw_id(
      cr, 
      style, 
      "#prev_robot", 
      unit, 
      scale, 
      robot->line, 
      robot->column, 
      -M_PI/2*robot->status,
      0
      ) ;
  /* TODO handle dead robot */
}

/**** Board ****/

static void draw_board_background (
    cairo_t* cr,
    RsvgHandle* style,
    double unit,
    double scale,
    const RRBoard* board
    )
{
  /* draw tile backgrounds */
  for(unsigned int t = 0; t < board->tile_size; ++t)
  {
    draw_tile_background(cr, style, unit, scale, board->tiles + t) ;
  }
}

static void draw_board_tiles (
    cairo_t* cr,
    RsvgHandle* style,
    double unit,
    double scale,
    const RRBoard* board
    )
{
  /* draw tile symbols */
  for(unsigned int t = 0; t < board->tile_size; ++t)
  {
    draw_tile_type(cr, style, unit, scale, board->tiles + t) ;
  }
}

/**** GTK Display ****/

/** Application data **/

typedef struct {
  const RRBoard* board ;
  RRRobot* robot ;
  RRRobot* prev_robot ;
  RsvgHandle* style ; /* unable to set it const due to Rsvg API */
} DrawData ;

/** Callbacks **/

/* Types */

typedef struct {
  guint state ;
  int xorigin ;
  int yorigin ;

  int line_translate ;
  int column_translate ;
  double zoom ;
} BoardNavigator ;

/* board drawing */

static gboolean draw_board_callback(
    GtkWidget* widget,
    cairo_t*   cr,
    gpointer   data
    )
{
  /* recover data */
  const RRBoard* board = 
    (const RRBoard*) g_object_get_data(G_OBJECT(widget), "board") ;
  RsvgHandle* style = 
    (RsvgHandle*) g_object_get_data(G_OBJECT(widget), "style") ;
  const RRRobot* robot = 
    (const RRRobot*) g_object_get_data(G_OBJECT(widget), "robot") ;
  const RRRobot* prev_robot = 
    (const RRRobot*) g_object_get_data(G_OBJECT(widget), "prev_robot") ;
  const BoardNavigator* navigator =
    (const BoardNavigator*) g_object_get_data(G_OBJECT(widget), "navigator") ;

  /* drawing area size */
  const double draw_width  = gtk_widget_get_allocated_width(widget) ;
  const double draw_height = gtk_widget_get_allocated_height(widget) ;

  /* size of the svg elements */
  RsvgDimensionData svg_dim ;
  rsvg_handle_get_dimensions(style, &svg_dim) ;

  /* compute the scale */
  static const unsigned int min_size = 6 ;
  const double min_scalex = draw_width / (min_size * svg_dim.width) ;
  const double min_scaley = draw_height / (min_size * svg_dim.height) ;
  const double scalex = 
    fmin(min_scalex,draw_width / (board->width * svg_dim.width)) ;
  const double scaley = 
    fmin(min_scaley,draw_height / (board->height * svg_dim.height)) ;
  double scale = fmin(scalex, scaley) ;

  /* compute the unit length */
  double unit = 
    scalex < scaley ?
    scalex * svg_dim.width :
    scaley * svg_dim.height ;

  /* draw the elements */
  if(navigator) {
    cairo_save(cr) ;
    cairo_translate(cr, draw_width / 2, draw_height / 2) ;
    cairo_scale(cr, navigator->zoom, navigator->zoom) ;
    cairo_translate(cr, -draw_width / 2, -draw_height / 2) ;
    cairo_translate(
        cr, 
        navigator->column_translate / navigator->zoom, 
        navigator->line_translate / navigator->zoom
        ) ;
  }

  draw_board_background(cr, style, unit, scale, board) ;
  if(robot) 
    draw_robot(cr, style, unit, scale, robot) ;
  if(prev_robot) 
    draw_previous_robot(cr, style, unit, scale, prev_robot) ;
  draw_board_tiles(cr, style, unit, scale, board) ;

  if(navigator) {
    cairo_restore(cr) ;
  }

  return FALSE ;
}

/* mouse robot placement */

static void viewer_button_pressed_handle(
    GtkWidget* widget,
    GdkEventButton* event,
    gpointer user_data
    )
{
  /* recover data */
  RRRobot* robot = 
    (RRRobot*) g_object_get_data(G_OBJECT(widget), "robot") ;
  const RRBoard* board = 
    (const RRBoard*) g_object_get_data(G_OBJECT(widget), "board") ;
  RsvgHandle* style = 
    (RsvgHandle*) g_object_get_data(G_OBJECT(widget), "style") ;

  if(!robot) return ;

  if(event->button == 1)
  {
    /* drawing area size */
    const double draw_width  = gtk_widget_get_allocated_width(widget) ;
    const double draw_height = gtk_widget_get_allocated_height(widget) ;

    /* size of the svg elements */
    RsvgDimensionData svg_dim ;
    rsvg_handle_get_dimensions(style, &svg_dim) ;

    /* compute the scale */
    static const unsigned int min_size = 6 ;
    const double min_scalex = draw_width / (min_size * svg_dim.width) ;
    const double min_scaley = draw_height / (min_size * svg_dim.height) ;
    const double scalex = 
      fmin(min_scalex,draw_width / (board->width * svg_dim.width)) ;
    const double scaley = 
      fmin(min_scaley,draw_height / (board->height * svg_dim.height)) ;

    /* compute the unit length */
    double unit = 
      scalex < scaley ?
      scalex * svg_dim.width :
      scaley * svg_dim.height ;

    /* clicked tile coordinates */
    RRTile dummytile ;
    dummytile.line = event->y / unit ;
    dummytile.column = event->x / unit ;
    dummytile.type = RR_TILE_NONE ;

    /* check whether the tile exists */
    RRTile* target = tile_search(
        board->tiles, 
        board->tile_size, 
        &dummytile
        ) ;

    /* place the robot on the tile */
    robot->line = target->line ;
    robot->column = target->column ;

    /* revive the robot if necessary */
    if(robot->status == RR_ROBOT_DEAD)
    {
      robot->status = RR_ROBOT_N ;
    }

    gtk_widget_queue_draw(widget) ;
  }
}

static gboolean viewer_scroll_callback(
    GtkWidget* widget,
    GdkEventScroll* event,
    gpointer user_data
    )
{
  RRRobot* robot = 
    (RRRobot*) g_object_get_data(G_OBJECT(widget), "robot") ;

  if(!robot) return FALSE ;

  if(event->direction == GDK_SCROLL_DOWN)
  {
    if(robot->status != RR_ROBOT_DEAD)
    {
      robot->status = (RRRobotStatus) ((robot->status + 1) % 4) ;
    }
  }
  else if(event->direction == GDK_SCROLL_UP)
  {
    if(robot->status != RR_ROBOT_DEAD)
    {
      robot->status = (RRRobotStatus) ((robot->status + 3) % 4) ;
    }
  }
  gtk_widget_queue_draw(widget) ;

  return TRUE ;
}


/* keyboard shortcuts */

static gboolean window_key_press(
    GtkWidget* widget,
    GdkEventKey* event,
    gpointer user_data
    )
{
  if(event->keyval == GDK_KEY_q) 
  {
    gtk_widget_destroy(widget) ;
    return TRUE ;
  }
  return FALSE ;
}


static gboolean area_key_press(
    GtkWidget* widget,
    GdkEventKey* event,
    gpointer user_data
    )
{
  RRRobot* robot = 
    (RRRobot*) g_object_get_data(G_OBJECT(widget), "robot") ;
  RRRobot* prev_robot = 
    (RRRobot*) g_object_get_data(G_OBJECT(widget), "prev_robot") ;
  const RRBoard* board = 
    (const RRBoard*) g_object_get_data(G_OBJECT(widget), "board") ;

  if(!robot) return FALSE ;

  if(event->keyval == GDK_KEY_1)
  {
    if(prev_robot)
    {
      memcpy(prev_robot, robot, sizeof(RRRobot)) ;
    }
    rr_board_play(*board, *robot, RR_MOVE_FORWARD_1) ;
    gtk_widget_queue_draw(widget) ;
    return TRUE ;
  }

  if(event->keyval == GDK_KEY_2)
  {
    if(prev_robot)
    {
      memcpy(prev_robot, robot, sizeof(RRRobot)) ;
    }
    rr_board_play(*board, *robot, RR_MOVE_FORWARD_2) ;
    gtk_widget_queue_draw(widget) ;
    return TRUE ;
  }

  if(event->keyval == GDK_KEY_3)
  {
    if(prev_robot)
    {
      memcpy(prev_robot, robot, sizeof(RRRobot)) ;
    }
    rr_board_play(*board, *robot, RR_MOVE_FORWARD_3) ;
    gtk_widget_queue_draw(widget) ;
    return TRUE ;
  }

  if(event->keyval == GDK_KEY_4)
  {
    if(prev_robot)
    {
      memcpy(prev_robot, robot, sizeof(RRRobot)) ;
    }
    rr_board_play(*board, *robot, RR_MOVE_BACKWARD_1) ;
    gtk_widget_queue_draw(widget) ;
    return TRUE ;
  }

  if(event->keyval == GDK_KEY_l)
  {
    if(prev_robot)
    {
      memcpy(prev_robot, robot, sizeof(RRRobot)) ;
    }
    rr_board_play(*board, *robot, RR_TURN_LEFT) ;
    gtk_widget_queue_draw(widget) ;
    return TRUE ;
  }

  if(event->keyval == GDK_KEY_r)
  {
    if(prev_robot)
    {
      memcpy(prev_robot, robot, sizeof(RRRobot)) ;
    }
    rr_board_play(*board, *robot, RR_TURN_RIGHT) ;
    gtk_widget_queue_draw(widget) ;
    return TRUE ;
  }

  if(event->keyval == GDK_KEY_u)
  {
    if(prev_robot)
    {
      memcpy(prev_robot, robot, sizeof(RRRobot)) ;
    }
    rr_board_play(*board, *robot, RR_U_TURN) ;
    gtk_widget_queue_draw(widget) ;
    return TRUE ;
  }

  return FALSE ;
}

/* GUI construction */

static void board_show_activate (
    GtkApplication* app,
    gpointer        user_data
    )
{
  const DrawData* data = (const DrawData*) user_data ;

  /* create the window */
  GtkWidget *window;
  window = gtk_application_window_new (app);
  gtk_window_set_title (GTK_WINDOW (window), "Robo Rally Viewer");
  gtk_window_set_default_size (GTK_WINDOW (window), 512, 512);
  g_signal_connect(
      window, 
      "key-press-event", 
      G_CALLBACK(window_key_press), 
      user_data
      ) ;

  /* create the drawing area and append it to the window */
  GtkWidget *draw_area = gtk_drawing_area_new() ;
  g_object_set_data(
      G_OBJECT(draw_area), 
      "board", 
      (RRBoard*) data->board /* const is back at recovery */
      ) ;
  g_object_set_data(
      G_OBJECT(draw_area), 
      "style", 
      data->style
      ) ;
  g_object_set_data(
      G_OBJECT(draw_area), 
      "robot", 
      data->robot
      ) ;
  g_object_set_data(
      G_OBJECT(draw_area), 
      "prev_robot", 
      data->prev_robot
      ) ;
  gtk_container_add(GTK_CONTAINER(window), draw_area) ;

  /* signals */
  g_signal_connect(
      draw_area, 
      "draw", 
      G_CALLBACK(draw_board_callback), 
      user_data
      ) ;
  gtk_widget_add_events(GTK_WIDGET(draw_area), GDK_BUTTON_PRESS_MASK);
  g_signal_connect(
      draw_area, 
      "button-press-event", 
      G_CALLBACK(viewer_button_pressed_handle), 
      user_data
      ) ;
  gtk_widget_add_events(GTK_WIDGET(draw_area), GDK_SCROLL_MASK);
  g_signal_connect(
      draw_area, 
      "scroll-event", 
      G_CALLBACK(viewer_scroll_callback), 
      user_data
      ) ;
  gtk_widget_add_events(GTK_WIDGET(draw_area), GDK_KEY_PRESS_MASK) ;
  gtk_widget_set_can_focus(GTK_WIDGET(draw_area), TRUE) ;
  g_signal_connect(
      draw_area, 
      "key-press-event", 
      G_CALLBACK(area_key_press), 
      user_data
      ) ;
  
  /* show all the elements */
  gtk_widget_show_all (window);
}

/**** GTK editor ****/

/** Editor data **/

struct EditorData_s ;

typedef struct {
  RRTileType current_type ;
  RRTileType min_type ;
  RRTileType max_type ;
  struct EditorData_s* editor_data ;
  GtkWidget* button ;
} EditorToolData ;

typedef struct EditorData_s {
  RRBoard* board ;
  unsigned int alloc_size ;
  BoardNavigator navigator ;
  EditorToolData* current_tool ;
  RsvgHandle* style ; /* unable to set it const due to Rsvg API */
} EditorData ;

/** Utilities **/

/* sorting tiles putting NONE tiles in the end */

static void tile_sort_tidy (
    RRTile* tile_start,
    unsigned int tile_size
    )
{
  /* sort the tiles */
  qsort(tile_start, tile_size, sizeof(RRTile), tile_compare_none) ;
}

/* counting true tiles (not NONE) in the end of a sorted tile array */

static unsigned int tile_count_real(
    const RRTile* tile_start,
    unsigned int tile_size
    )
{
  /* count NONE tiles in the end */
  unsigned int none_tiles = 0 ;
  while(
      none_tiles < tile_size &&
      tile_start[tile_size-1-none_tiles].type == RR_TILE_NONE)
  {
    ++none_tiles ;
  }

  return tile_size - none_tiles ;
}

/* tile copy */
static void tile_copy(
    const RRTile* source,
    RRTile* target
    )
{
  memcpy(target, source, sizeof(RRTile)) ;
}

/* inserting a new tile, supposing allocation is sufficient */

static void tile_insert(
    RRTile* tile_start,
    unsigned int tile_size,
    const RRTile* new_tile
    )
{
  /* displace all elements above */
  RRTile* insert_pos = tile_start + tile_size ;
  while(insert_pos > tile_start && tile_compare(insert_pos-1, new_tile) > 0)
  {
    tile_copy(insert_pos-1, insert_pos) ;
    --insert_pos ;
  }

  /*insert the new tile*/
  tile_copy(new_tile, insert_pos) ;
}

/* edit a tile in a board, inserting if necessary */

static void board_edit(
    EditorData* editor_data,
    RRTile* new_tile
    )
{
  /* shortcut */
  RRBoard* board = editor_data->board ;

  /* search the tile in the board */
  RRTile* target = tile_search(
      board->tiles, 
      board->tile_size, 
      new_tile
      ) ;

  /* if the tile exists, edit it */
  if(target) 
  {
    target->type = new_tile->type ;
  }
  else
  {
    /* check whether the board has room for insertion */
    if(editor_data->alloc_size > board->tile_size)
    {
      /* natural insertion */
      tile_insert(board->tiles, board->tile_size, new_tile) ;
      ++board->tile_size ;
    }
    else
    {
      /* try first to remove useless tiles */
      tile_sort_tidy(board->tiles, board->tile_size) ;
      board->tile_size = tile_count_real(board->tiles, board->tile_size) ;

      /* check whether useless tiles were removed */
      if(editor_data->alloc_size > board->tile_size)
      {
        printf("insertion\n") ;
        /* natural insertion */
        tile_insert(board->tiles, board->tile_size, new_tile) ;
        ++board->tile_size ;
      }
      else
      {
        /* reallocate with twice the space */
        RRTile* tmp = (RRTile*) realloc(
            board->tiles, 
            2*editor_data->alloc_size*sizeof(RRTile)
            ) ;
        if(!tmp)
        {
          /* twice the space was too much, try with one more tile */
          tmp = (RRTile*) realloc(
              board->tiles, 
              (editor_data->alloc_size+1)*sizeof(RRTile)
              ) ;
          if(!tmp)
          {
            /* all reallocations failed */
            printf("unable to allocate space for new tiles, remove some\n") ;
            return ;
          }
          else
          {
            /* update the allocation size */
            editor_data->alloc_size += 1 ;
          }
        }
        else
        {
          /* update the allocation size */
          editor_data->alloc_size *= 2 ;
        }
        /* update the tiles pointer and insert the new tile */
        board->tiles = tmp ;
        tile_insert(board->tiles, board->tile_size, new_tile) ;
        ++board->tile_size ;
      }
    }
  }
}

static void board_normalize(
    RRBoard* board
    )
{
  /* sort the tiles moving none tiles to the end */
  tile_sort_tidy(board->tiles, board->tile_size) ;

  /* remove none tiles */
  board->tile_size = tile_count_real(board->tiles, board->tile_size) ;
  
  /* check whether tiles remain */
  if(board->tile_size)
  {
    /* shrink the tiles array */
    RRTile* tmp = (RRTile*) realloc(board->tiles, board->tile_size*sizeof(RRTile)) ;
    if(!tmp)
    {
      printf("unable to shrink tile memory, weird but not fatal\n") ;
    }
    else
    {
      board->tiles = tmp ;
    }
    /* normalize tile coordinates */
    int lmin = board->tiles[0].line ;
    int lmax = board->tiles[0].line ;
    int cmin = board->tiles[0].column ;
    int cmax = board->tiles[0].column ;

    for(
        RRTile* t = board->tiles + 1 ; 
        t < board->tiles + board->tile_size ; 
        ++t
        )
    {
      lmin = t->line < lmin ? t->line : lmin ;
      lmax = t->line > lmax ? t->line : lmax ;
      cmin = t->column < cmin ? t->column : cmin ;
      cmax = t->column > cmax ? t->column : cmax ;
    }

    for(
        RRTile* t = board->tiles ; 
        t < board->tiles + board->tile_size ; 
        ++t
        )
    {
      t->line -= lmin ;
      t->column -= cmin ;
    }

    board->width = lmax - lmin ;
    board->height = cmax - cmin ;
  }
  else
  {
    free(board->tiles) ;
    board->tiles = NULL ;
  }
}

/** Callbacks **/

/* Tile buttons */

static void draw_tool_callback(
    GtkWidget* widget,
    cairo_t*   cr,
    gpointer   user_data
    )
{
  /* tool data */
  EditorToolData* data = (EditorToolData*) user_data ;

  /* drawing area size */
  const double draw_width  = gtk_widget_get_allocated_width(widget) ;

  /* size of the svg elements */
  RsvgDimensionData svg_dim ;
  rsvg_handle_get_dimensions(data->editor_data->style, &svg_dim) ;

  /* compute the scale */
  const double scale = draw_width / svg_dim.width ;

  /* create the tile corresponding to the tool*/
  RRTile tooltile ;
  tooltile.line = 0 ;
  tooltile.column = 0 ;
  tooltile.type = data->current_type ;

  /* draw the tile as an icon */
  draw_tile_background(
      cr,
      data->editor_data->style,
      draw_width,
      scale,
      &tooltile
      ) ;
  draw_tile_type(
      cr,
      data->editor_data->style,
      draw_width,
      scale,
      &tooltile
      ) ;
}

static gboolean editor_scroll_callback(
    GtkWidget* widget,
    GdkEventScroll* event,
    gpointer user_data
    )
{
  EditorData* data = (EditorData*) user_data ;
  EditorToolData* t_data = data->current_tool ;

  /* Check scroll direction */
  if(event->direction == GDK_SCROLL_DOWN)
  {
    /* increment current type */
    if(t_data->current_type == t_data->max_type)
    {
      t_data->current_type = t_data->min_type ;
    }
    else
    {
      t_data->current_type = (RRTileType) (t_data->current_type + 1) ;
    }

    /* redraw */
    gtk_widget_queue_draw(t_data->button) ;

    return TRUE ;
  }
  else if(event->direction == GDK_SCROLL_UP)
  {
    /* decrement current type */
    if(t_data->current_type == t_data->min_type)
    {
      t_data->current_type = t_data->max_type ;
    }
    else
    {
      t_data->current_type = (RRTileType) (t_data->current_type - 1) ;
    }

    /* redraw */
    gtk_widget_queue_draw(t_data->button) ;

    return TRUE ;
  }
  return FALSE ;
}

static void tool_clicked_handle(
    GtkWidget* widget,
    gpointer user_data
    )
{
  /* filter event : also triggered on the button previously active */
  if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget)))
  {
    EditorToolData* data = (EditorToolData*) user_data ;
    /* change the current tool */
    data->editor_data->current_tool = data ;
  }
}

static void editor_button_pressed_handle(
    GtkWidget* widget,
    GdkEventButton* event,
    gpointer user_data
    )
{
  /* recover editor data */
  EditorData* data = (EditorData*) user_data ;

  if(event->button == 1)
  {
    /* drawing area size */
    const double draw_width  = gtk_widget_get_allocated_width(widget) ;
    const double draw_height = gtk_widget_get_allocated_height(widget) ;

    /* size of the svg elements */
    RsvgDimensionData svg_dim ;
    rsvg_handle_get_dimensions(data->style, &svg_dim) ;

    /* compute the scale */
    static const unsigned int min_size = 6 ;
    const double min_scalex = draw_width / (min_size * svg_dim.width) ;
    const double min_scaley = draw_height / (min_size * svg_dim.height) ;
    const double scalex = 
      fmin(min_scalex,draw_width / (data->board->width * svg_dim.width)) ;
    const double scaley = 
      fmin(min_scaley,draw_height / (data->board->height * svg_dim.height)) ;

    /* compute the unit length */
    double unit = 
      scalex < scaley ?
      scalex * svg_dim.width :
      scaley * svg_dim.height ;

    /* initial position */
    double line = event->y ;
    double column = event->x ;

    /* revert the navigator translation */
    line -= data->navigator.line_translate ;
    column -= data->navigator.column_translate ;

    /*revert the navigator scale*/
    line -= draw_height / 2 ;
    column -= draw_width / 2 ;
    line /= data->navigator.zoom ;
    column /= data->navigator.zoom ;
    line += draw_height / 2 ;
    column += draw_width / 2 ;

    /* tile coordinates */
    RRTile dummytile ;
    dummytile.line = line > 0 ? line / unit : line / unit - 1 ;
    dummytile.column = column > 0 ? column / unit : column / unit - 1 ;
    dummytile.type = data->current_tool->current_type ;

    /* edit the board */
    board_edit(data, &dummytile) ;

    /* redraw the board */
    gtk_widget_queue_draw(widget) ;

    //printf("board clicked at %d %d\n", line, column) ;
  }
  else if(event->button == 3)
  {
    data->navigator.state = event->state ;
    data->navigator.xorigin = event->x ;
    data->navigator.yorigin = event->y ;
  }
}

static gboolean editor_motion_handle(
    GtkWidget* widget,
    GdkEventMotion* event,
    gpointer* user_data
    )
{
  /* only motions with button 3 pressed are interesting */
  if(!(event->state & GDK_BUTTON3_MASK)) return FALSE ;

  EditorData* data = (EditorData*) user_data ;

  if(data->navigator.state & GDK_SHIFT_MASK)
  {
    data->navigator.zoom *= pow(1.005, event->y- data->navigator.yorigin) ;
    data->navigator.yorigin = event->y ;
  }
  else
  {
    data->navigator.column_translate += event->x - data->navigator.xorigin ;
    data->navigator.line_translate += event->y - data->navigator.yorigin ;
    data->navigator.xorigin = event->x ;
    data->navigator.yorigin = event->y ;
  }
  gtk_widget_queue_draw(widget) ;

  return TRUE ;
}

/** GUI construction **/

/* tile button creation */

static EditorToolData* tool_insert(
    RRTileType min_type,
    RRTileType max_type,
    EditorData* e_data,
    GtkWidget *toolbar
    )
{
  /* Recover radio group for the new button */
  GtkRadioButton* group = NULL ;
  if(gtk_toolbar_get_n_items(GTK_TOOLBAR(toolbar))) {
    /* list of items in the toolbar */
    GList* elements = 
      gtk_container_get_children(GTK_CONTAINER(toolbar)) ;
    /* first item in the toolbar */
    GList* first_elements = 
      gtk_container_get_children(GTK_CONTAINER(elements->data)) ;
    /* button contained in the first item */
    group = GTK_RADIO_BUTTON(first_elements->data) ;
  }
  /* Tool image */
  GtkWidget* icon = NULL ;

  if(min_type != RR_TILE_NONE) {
    /* the image comes from a tile */
    icon = gtk_drawing_area_new() ;
  } else {
    /* default image for tile suppression tool */
    icon = gtk_image_new_from_icon_name("edit-delete", GTK_ICON_SIZE_LARGE_TOOLBAR) ;
  }
  /* setup icon size */
  gtk_widget_set_size_request(icon, 35, 35) ;
  gtk_widget_set_halign(icon, GTK_ALIGN_CENTER) ;
  gtk_widget_set_valign(icon, GTK_ALIGN_CENTER) ;

  /* toolbar item */
  GtkToolItem* item = gtk_tool_item_new() ;
  gtk_toolbar_insert(GTK_TOOLBAR(toolbar), item, -1) ;

  /* radio button for the tool */
  GtkWidget* button = gtk_radio_button_new_from_widget(group) ;
  gtk_container_add(GTK_CONTAINER(item), button) ;

  /* setup button icon */
  gtk_button_set_image(GTK_BUTTON(button), icon) ;

  /* button style : no relief and no radio circle in front of icon */
  gtk_button_set_relief(GTK_BUTTON(button), GTK_RELIEF_NONE) ;
  gtk_toggle_button_set_mode(GTK_TOGGLE_BUTTON(button), FALSE) ;
  
  /* tool data */
  EditorToolData* data = (EditorToolData*) malloc(sizeof(EditorToolData)) ;
  data->current_type = min_type ;
  data->min_type = min_type ;
  data->max_type = max_type ;
  data->editor_data = e_data ;
  data->button = button ;

  /* ensure the data is cleaned up with the tool */
  g_object_set_data_full(
      G_OBJECT(button), 
      "roborally", 
      data, 
      free
      ) ;

  /* Events */
  if(min_type != RR_TILE_NONE)
  {
    /* the none tile is not drawn since its icon is not a tile */
    g_signal_connect(
        icon, 
        "draw", 
        G_CALLBACK(draw_tool_callback), 
        data
        ) ;
  }

  /* clicked event to switch tool */
  g_signal_connect(
      button, 
      "clicked", 
      G_CALLBACK(tool_clicked_handle), 
      data
      ) ;

  return data ;
}

static void board_edit_activate (
    GtkApplication* app,
    gpointer        user_data
    )
{
  EditorData* data = (EditorData*) user_data ;

  /* create the window */
  GtkWidget *window;
  window = gtk_application_window_new (app);
  gtk_window_set_title (GTK_WINDOW (window), "Robo Rally Editor");
  g_signal_connect(
      window, 
      "key-press-event", 
      G_CALLBACK(window_key_press), 
      NULL
      ) ;

  /* scroll event to rotate / mirror the current tool */
  gtk_widget_add_events(window, GDK_SCROLL_MASK);
  g_signal_connect(
      window, 
      "scroll-event", 
      G_CALLBACK(editor_scroll_callback), 
      data
      ) ;

  /* split between tools and board */
  GtkWidget* grid = gtk_grid_new() ;
  gtk_container_add(GTK_CONTAINER(window), grid) ;

  /* create the drawing area and append it to the window */
  GtkWidget* draw_area = gtk_drawing_area_new() ;
  gtk_grid_attach(GTK_GRID(grid), draw_area, 0, 0, 1, 1) ;
  gtk_widget_set_size_request(draw_area, 512, 512) ;
  gtk_widget_set_hexpand(draw_area, TRUE) ;
  gtk_widget_set_vexpand(draw_area, TRUE) ;
  g_object_set_data(
      G_OBJECT(draw_area), 
      "board", 
      (RRBoard*) data->board
      ) ;
  g_object_set_data(
      G_OBJECT(draw_area), 
      "style", 
      data->style
      ) ;
  g_object_set_data(
      G_OBJECT(draw_area),
      "navigator",
      &(data->navigator)
      ) ;
  g_signal_connect(
      draw_area, 
      "draw", 
      G_CALLBACK(draw_board_callback), 
      data
      ) ;
  gtk_widget_add_events(GTK_WIDGET(draw_area), GDK_BUTTON_PRESS_MASK);
  g_signal_connect(
      draw_area, 
      "button-press-event", 
      G_CALLBACK(editor_button_pressed_handle), 
      data
      ) ;
  gtk_widget_add_events(GTK_WIDGET(draw_area), GDK_POINTER_MOTION_MASK);
  g_signal_connect(
      draw_area, 
      "motion-notify-event", 
      G_CALLBACK(editor_motion_handle), 
      data
      ) ;

  /* create a toolbar for the various tiles */
  GtkWidget* tilebar = gtk_toolbar_new() ;
  gtk_toolbar_set_style(GTK_TOOLBAR(tilebar), GTK_TOOLBAR_ICONS) ;
  gtk_toolbar_set_show_arrow(GTK_TOOLBAR(tilebar), FALSE) ;
  gtk_orientable_set_orientation(GTK_ORIENTABLE(tilebar), GTK_ORIENTATION_VERTICAL) ;
  gtk_grid_attach_next_to(GTK_GRID(grid), tilebar, draw_area, GTK_POS_RIGHT, 1, 1) ;

  /* create tile buttons */
  data->current_tool = tool_insert(
      RR_TILE_NOTHING, 
      RR_TILE_NOTHING, 
      data,
      tilebar
      ) ;

  tool_insert(
      RR_TILE_ROTATE_RIGHT, 
      RR_TILE_ROTATE_LEFT, 
      data,
      tilebar
      ) ;

  tool_insert(
      RR_TILE_BELT_E, 
      RR_TILE_BELT_S, 
      data,
      tilebar
      ) ;

  tool_insert(
      RR_TILE_TURN_SE, 
      RR_TILE_TURN_ES, 
      data,
      tilebar
      ) ;

  tool_insert(
      RR_TILE_JOIN_S_TO_E, 
      RR_TILE_JOIN_E_TO_S, 
      data,
      tilebar
      ) ;

  tool_insert(
      RR_TILE_JOIN_NS_TO_E, 
      RR_TILE_JOIN_EW_TO_S, 
      data,
      tilebar
      ) ;

  tool_insert(
      RR_TILE_FAST_BELT_E, 
      RR_TILE_FAST_BELT_S, 
      data,
      tilebar
      ) ;

  tool_insert(
      RR_TILE_FAST_TURN_SE, 
      RR_TILE_FAST_TURN_ES, 
      data,
      tilebar
      ) ;

  tool_insert(
      RR_TILE_FAST_JOIN_S_TO_E, 
      RR_TILE_FAST_JOIN_E_TO_S, 
      data,
      tilebar
      ) ;

  tool_insert(
      RR_TILE_FAST_JOIN_NS_TO_E, 
      RR_TILE_FAST_JOIN_EW_TO_S, 
      data,
      tilebar
      ) ;

  tool_insert(
      RR_TILE_NONE, 
      RR_TILE_NONE, 
      data,
      tilebar
      ) ;

  /* show all the elements */
  gtk_widget_show_all(window);
}

/**** Public interface ****/

void rr_board_show(
    const RRBoard& board_in,
    const char* style,
    RRRobot& robot_in,
    RRRobot& previous_robot_in
    )
{
  const RRBoard* board = &board_in ;
  RRRobot* robot = &robot_in ;
  RRRobot* previous_robot = &previous_robot_in ;

  /* print help */
  printf("click : place the robot\n") ;
  printf("scroll : rotate the robot\n") ;
  printf("1 : move forward 1\n") ;
  printf("2 : move forward 2\n") ;
  printf("3 : move forward 3\n") ;
  printf("4 : move backward 1\n") ;
  printf("l : turn left\n") ;
  printf("r : turn right\n") ;
  printf("u : u turn\n") ;
  printf("q : quit\n") ;

  GtkApplication *app ;
  int status ;
  DrawData data ;

  data.board = board ;
  data.robot = robot ;
  data.prev_robot = previous_robot ;

  /* load the svg file for the element styles */
  GError* svg_error = NULL ;
  data.style = rsvg_handle_new_from_file(style, &svg_error) ;
  if(svg_error) {
    printf("unable to read style file\n") ;
    return ;
  }

  app = gtk_application_new ("org.gtk.roborally_viewer", G_APPLICATION_NON_UNIQUE) ;
  g_signal_connect (
      app, 
      "activate", 
      G_CALLBACK (board_show_activate), 
      &data
      ) ;
  status = g_application_run (G_APPLICATION (app), 0, NULL) ;
  g_object_unref (app) ;
  g_object_unref(data.style) ;

  if(status)
  {
    printf("GTK application returned with status %d\n", status) ;
  }
}

void rr_board_edit(
    RRBoard& board_in,
    const char* style
    )
{
  RRBoard* board = &board_in ;

  /* print help */
  printf("click : place the selected tile\n") ;
  printf("scroll : rotate the selected tile\n") ;
  printf("right click + move : scroll the board\n") ;
  printf("shift + right click + move : zoom the board\n") ;
  printf("q : save and quit\n") ;

  GtkApplication *app ;
  int status ;
  EditorData data ;

  data.board = board ;
  data.alloc_size = board->tile_size ;
  data.current_tool = NULL ;
  data.navigator.state = 0 ;
  data.navigator.xorigin = 0 ;
  data.navigator.yorigin = 0 ;
  data.navigator.line_translate = 0 ;
  data.navigator.column_translate = 0 ;
  data.navigator.zoom = 1 ;

  /* initialize if necessary */
  if(board->tile_size == 0)
  {
    /* the board has no tiles allocate tiles anyway to store additional ones*/
    static const unsigned int default_alloc = 10 ; /* do not use 0 */
    RRTile* tmp = (RRTile*) malloc(default_alloc*sizeof(RRTile)) ;
    if(!tmp)
    {
      printf("unable to allocate storage for any tile\n") ;
      return ;
    }
    board->tiles = tmp ;
    data.alloc_size = default_alloc ;
  }

  /* load the svg file for the element styles */
  GError* svg_error = NULL ;
  data.style = rsvg_handle_new_from_file(style, &svg_error) ;
  if(svg_error) {
    printf("unable to read style file\n") ;
    return ;
  }

  app = gtk_application_new ("org.gtk.roborally_editor", G_APPLICATION_NON_UNIQUE) ;
  g_signal_connect (
      app, 
      "activate", 
      G_CALLBACK (board_edit_activate), 
      &data
      ) ;

  char ed_str[7] ;
  std::strcpy(ed_str, "editor") ;

  char* options[] = {
    ed_str
  } ;

  status = g_application_run (G_APPLICATION (app), 1, options) ;
  g_object_unref (app) ;
  g_object_unref(data.style) ;

  board_normalize(board) ;

  if(status)
  {
    printf("GTK application returned with status %d\n", status) ;
  }
}
