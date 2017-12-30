#include <stdlib.h>
#include <stdio.h>
#include <libgen.h>
#include <pthread.h>

#include "sim_avr.h"
#include "avr_ioport.h"
#include "avr_spi.h"
#include "avr_timer.h"
#include "sim_elf.h"
#include "sim_gdb.h"
#include "sim_vcd_file.h"

avr_t *avr = NULL;

int window_identifier;

static void *avr_run_thread(void * ignore) {
  while (1) {
    avr_run(avr);
  }
  return NULL;
}


int main(int argc, char *argv[])
{
  elf_firmware_t f;
  const char * fname =  "atmega328_arg.axf";
  elf_read_firmware(fname, &f);

  printf("firmware %s f=%d mmcu=%s\n", fname, (int)f.frequency, f.mmcu);

  avr = avr_make_mcu_by_name(f.mmcu);
  if (!avr) {
    fprintf(stderr, "%s: AVR '%s' not known\n", argv[0], f.mmcu);
    exit(1);
  }
  avr_init(avr);
  avr_load_firmware(avr, &f);

  // even if not setup at startup, activate gdb if crashing
  avr->gdb_port = 1234;
  if (1) {
    avr->state = cpu_Stopped;
    avr_gdb_init(avr);
  }

  pthread_t run;
  pthread_create(&run, NULL, avr_run_thread, NULL);

  // glutMainLoop();
}
