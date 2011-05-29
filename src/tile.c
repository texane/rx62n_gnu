#include "config.h"
#include <sys/types.h>
#include "tile.h"


/* find a free tile of the same color in the neighborhood */

int find_free_neighbor_tile
(const unsigned int* tiles, unsigned int is_red, unsigned int* x, unsigned int* y)
{
  /* north, clockwise */
  static const int dirs[8][2] =
  {
    {  0, -1 },
    {  1, -1 },
    {  1,  0 },
    {  1,  1 },
    {  0,  1 },
    { -1,  1 },
    { -1,  0 },
    { -1, -1 }
  };

  /* translate to tile repere */

  static const size_t ndirs = sizeof(dirs) / sizeof(dirs[0]);

  size_t i;
  for (i = 0; i < ndirs; ++i)
  {
    const int nx = (int)*x + dirs[i][0];
    const int ny = (int)*y + dirs[i][1];

    if ((nx == -1) || (nx >= (int)tiles_per_row))
      continue ;
    else if ((ny == -1) || (ny >= (int)tiles_per_col))
      continue ;
    else if (is_tile_red(nx, ny) != is_red)
      continue ;
    else if (is_tile_used(tiles, nx, ny))
      continue ;

    /* found a free is_red tile */
    *x = (unsigned int)nx;
    *y = (unsigned int)ny;

    return 0;
  }

  return -1;
}


#if 0 /* UNUSED */

#include <stdio.h>

/* print the tile states */

static inline char tile_to_char(unsigned int tile)
{
  if ((tile & TILE_FLAG_SEEN) == 0)
  {
    return '?';
  }
  else if (tile & TILE_FLAG_USED)
  {
    if (tile & TILE_FLAG_RED)
      return 'r';
    return 'b';
  }
  return ' ';
}

void print_tiles(const unsigned int* tiles)
{
  for (size_t i = 0; i < tiles_per_row; ++i)
  {
    for (size_t j = 0; j < tiles_per_col; ++j)
      printf("%c", tile_to_char(get_const_tile_at(tiles, i, j)));
    printf("\n");
  }
  printf("\n");
}

#endif /* UNUSED */
