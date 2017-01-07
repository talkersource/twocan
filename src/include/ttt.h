#ifndef TTT_H
#define TTT_H

#ifdef TTT_C
/* tic tac toe is a 3 x 3 board....
 *
 *         1         |        2       |      3
 *
 *         4         |        5       |      6
 *
 *         7         |        8       |      9
 *
 * this is layed out in memory as...
 * 1 = 0x0001
 * 2 = 0x0002
 * 3 = 0x0004
 * 4 = 0x0008
 * 5 = 0x0010
 * 6 = 0x0020
 * 7 = 0x0030
 * 8 = 0x0040
 * 9 = 0x0100
 *
 */

/* 1 - 9 is easier to understand than 0 - 8 */
# define TTT_BIT(x) (1 << ((x) - 1))

/* masks in the form... [1, 2, 3] -- so it matches BITS */
# define TTT_MASK_H(x) (TTT_BIT(1 + ((x - 1) * 3)) | \
 TTT_BIT(2 + ((x - 1) * 3)) | TTT_BIT(3 + ((x - 1) * 3)))

# define TTT_MASK_V(x) \
 (TTT_BIT(0 + (x)) | TTT_BIT(3 + (x)) | TTT_BIT(6 + (x)))

# define TTT_MASK_D_1() (TTT_BIT(1) | TTT_BIT(5) | TTT_BIT(9))
# define TTT_MASK_D_2() (TTT_BIT(3) | TTT_BIT(5) | TTT_BIT(7))

# define TTT_MASK_FULL() (TTT_MASK_H(1) | TTT_MASK_H(2) | TTT_MASK_H(3))

# define TTT_IS_DRAW(x) (((x) & TTT_MASK_FULL()) == TTT_MASK_FULL())

# define TTT_WON_H(x, c) (((x) & TTT_MASK_H(c)) == TTT_MASK_H(c))

# define TTT_WON_V(x, c) (((x) & TTT_MASK_V(c)) == TTT_MASK_V(c))

# define TTT_WON_D_1(x) (((x) & TTT_MASK_D_1()) == TTT_MASK_D_1())
# define TTT_WON_D_2(x) (((x) & TTT_MASK_D_2()) == TTT_MASK_D_2())

#endif

#endif /* TTT_H */
