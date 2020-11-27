
#include <stdint.h>
#include <stddef.h>
#include <atspi/atspi.h>
#include <iostream>
using namespace std;

void on_event(AtspiEvent *event, void *data)
{
    g_object_unref(event->source);
    g_free(event->type);
    g_value_unset(&event->any_data);
    g_free(event);
}

int init(void)
{
    int result = atspi_init();
    if (result != 0)
    {
        std::cout <<"init error"<<std::endl;
        return 0;
    }
      AtspiEventListener *listener = atspi_event_listener_new(on_event, NULL, NULL);
      if (listener)
      {
          std::cout<<"listener OK"<<std::endl;
          atspi_event_main();
      }
}

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
    init();
  return 0;
}



