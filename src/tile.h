#ifndef TILE_H_INCLUDED
# define TILE_H_INCLUDED


#include <string.h>


/* tile array dimension */
#define tiles_per_row 6
#define tiles_per_col 6
#define tile_count (tiles_per_col * tiles_per_row)


/* tile filags */
#define TILE_FLAG_SEEN (1 << 0)
#define TILE_FLAG_USED (1 << 1)
#define TILE_FLAG_RED (1 << 2)
#define TILE_FLAG_MEIN (1 << 2)


static inline void tile_to_world(unsigned int* x, unsigned int* y)
{
  /* translate from tile to world */
  *x = 450 + (*x) * 350 + 350 / 2;
  *y = (*y) * 350 + 350 / 2;
}

static inline void world_to_tile(unsigned int* x, unsigned int* y)
{
  /* translate from world to tile. assume x >= 450 */
  *x = ((*x) - 450) / 350;
  *y = (*y) / 350;
}

static inline unsigned int clamp_tile_x(unsigned int world_x)
{
  /* ensure world_x fits in is in tile range */
  return world_x < 500 ? 500 : (world_x > 2500 ? 2500 : world_x);
}

static inline void init_tiles(unsigned int* tiles)
{
  /* initialize an array of tile states */
  memset(tiles, 0, tile_count * sizeof(unsigned int));
}

static inline unsigned int* get_tile_at
(unsigned int* tiles, unsigned int x, unsigned int y)
{
  return &tiles[y * tiles_per_row + x];
}

static inline const unsigned int* get_const_tile_at
(const unsigned int* tiles, unsigned int x, unsigned int y)
{
  return &tiles[y * tiles_per_row + x];
}

static inline unsigned int is_tile_used_(const unsigned int* tile)
{
  return ((*tile) & TILE_FLAG_USED) ? 1 : 0;
}

static inline unsigned int is_tile_used
(const unsigned int* tiles, unsigned int x, unsigned int y)
{
  return is_tile_used_(get_const_tile_at(tiles, x, y));
}

static inline void set_tile_used_
(unsigned int* tile, unsigned int is_red)
{
  *tile |= TILE_FLAG_USED;
  if (is_red) *tile |= TILE_FLAG_RED;
}

static inline void set_tile_used
(unsigned int* tiles, unsigned int x, unsigned int y, unsigned int is_red)
{
  set_tile_used_(get_tile_at(tiles, x, y), is_red);
}

static inline void clear_tile_used_(unsigned int* tile)
{
  *tile &= ~(TILE_FLAG_USED | TILE_FLAG_RED);
}

static inline void clear_tile_used
(unsigned int* tiles, unsigned int x, unsigned int y)
{
  clear_tile_used_(get_tile_at(tiles, x, y));
}

static inline int is_tile_red(unsigned int x, unsigned int y)
{
  return (y & 1) ^ (x & 1); 
}

static inline void get_tile_xy
(unsigned int* tiles, unsigned int* tile, unsigned int* x, unsigned int* y)
{
  const size_t pos = tile - tiles;
  *x = pos / tiles_per_row;
  *y = pos % tiles_per_col;
}

/* non inlined */
int find_free_neighbor_tile(const unsigned int*, unsigned int, unsigned int*, unsigned int*);

#if CONFIG_DEBUG
void print_tiles(const unsigned int*);
#endif


#endif /* TILE_H_INCLUDED */
