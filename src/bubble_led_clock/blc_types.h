#ifndef BLC_TYPES_H
#define BLC_TYPES_H

// --- Shared Animation & Scene Types ---
enum AnimationType {
  SLOT_MACHINE,
  MATRIX
};

struct DisplayScene {
  const char* scene_name;
  const char* format_string;
  AnimationType animation_type;
  bool dots_with_previous;
  unsigned long duration_ms;
  unsigned long anim_param_1;
  unsigned long anim_param_2;
  float (*getDataValue)(); 
};

// --- Shared Access Point Form Types ---
// A generic struct to hold the text and value for a dropdown option
struct PrefSelectOption {
  const char* name;  // The user-friendly text (e.g., "Eastern Time")
  const char* value; // The value to save (e.g., "EST5EDT,M3.2.0...")
};

#endif // BLC_TYPES_H
