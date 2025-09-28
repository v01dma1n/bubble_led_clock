#ifndef BLC_ACCESS_POINT_MANAGER_H
#define BLC_ACCESS_POINT_MANAGER_H

#include <base_access_point_manager.h>

class BlcAccessPointManager : public BaseAccessPointManager {
public:
    BlcAccessPointManager(AppPreferences& prefs) : BaseAccessPointManager(prefs) {}

protected:
    // We override this to add our BLC-specific fields
    void initializeFormFields() override;
};

#endif // BLC_ACCESS_POINT_MANAGER_H