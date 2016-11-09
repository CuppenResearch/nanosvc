#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "segment.h"

int
main ()
{
  uint8_t succeeded = 0;
  uint8_t failed = 0;
  uint8_t skipped = 0;

  struct nsv_segment_t *segment;
  struct nsv_segment_cigar_overview_t overview;

  puts ("--------------------------- CIGAR TESTS ---------------------------");
  segment = nsv_segment_new ();
  if (segment == NULL)
    {
      puts ("  * Skipped simple parsing because of a memory allocation error.");
      skipped++;
    }
  else
    {
      segment->cigar = strdup ("133I44D2M675I16M");

      overview = nsv_segment_cigar_overview (segment);
      if (overview.insertions == 808
          && overview.deletions == 44
          && overview.alignment_matches == 18)
        {
          puts ("  * Simple parsing works fine.");
          succeeded++;
        }
      else
        {
          puts ("  * ERROR: Simple parsing failed.");
          failed++;
        }

      free (segment->cigar);
      segment->cigar = strdup ("11I6M2D34DD6I");

      overview = nsv_segment_cigar_overview (segment);
      if (overview.insertions == 17
          && overview.deletions == 36
          && overview.alignment_matches == 6)
        {
          puts ("  * Faulty parsing works fine.");
          succeeded++;
        }
      else
        {
          puts ("  * ERROR: Faulty parsing failed.");
          failed++;
        }
  
      nsv_segment_destroy (segment);
    }
  puts ("------------------------- END CIGAR TESTS -------------------------");

  printf ("\nSucceeded: %u\nFailed:    %u\nSkipped:   %u\n",
          succeeded, failed, skipped);

  return (failed == 0);
}
