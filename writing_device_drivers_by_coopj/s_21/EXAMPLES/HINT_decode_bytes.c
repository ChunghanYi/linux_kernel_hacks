/* **************** LDD:2.0 s_21/HINT_decode_bytes.c **************** */
/*
 * The code herein is: Copyright Jerry Cooperstein, 2012
 *
 * This Copyright is retained for the purpose of protecting free
 * redistribution of source.
 *
 *     URL:    http://www.coopj.com
 *     email:  coop@coopj.com
 *
 * The primary maintainer for this code is Jerry Cooperstein
 * The CONTRIBUTORS file (distributed with this
 * file) lists those known to have contributed to the source.
 *
 * This code is distributed under Version 2 of the GNU General Public
 * License, which you should have received with the source.
 *
 */
/* Change in X since last message */
dx = (signed char)(((byte_in[0] & 0x03) << 6) | (byte_in[1] & 0x3f));
/* Change in Y since last message */
dy = (signed char)(((byte_in[0] & 0x0c) << 4) | (byte_in[2] & 0x3f));
/* Is the left button pressed? */
left = (byte_in[0] & 0x20 ? 'L' : ' ');
/* Is the right button pressed? */
right = (byte_in[0] & 0x10 ? 'R' : ' ');
