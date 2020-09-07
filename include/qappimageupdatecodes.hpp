#ifndef QAPPIMAGE_UPDATE_CODES_HPP_INCLUDED
#define QAPPIMAGE_UPDATE_CODES_HPP_INCLUDED

struct QAppImageUpdateCodes {
struct Action {
enum : short {
	       None,
	       GetEmbeddedInfo,
	       CheckForUpdate,
	       Update,
	       UpdateWithGUI       
};
};

struct GuiFlag {
enum {
        ShowProgressDialog = 0x1,
        ShowBeforeProgress = 0x2,
        ShowUpdateConfirmationDialog = 0x4,
        ShowFinishedDialog = 0x8,
        ShowErrorDialog = 0x10,
        AlertWhenAuthorizationIsRequired = 0x20,
        NotifyWhenNoUpdateIsAvailable = 0x40,
        NoRemindMeLaterButton = 0x80,
        NoSkipThisVersionButton = 0x100,
        Default = ShowBeforeProgress |
                  ShowProgressDialog |
                  ShowUpdateConfirmationDialog |
                  ShowFinishedDialog   |
                  ShowErrorDialog |
                  NotifyWhenNoUpdateIsAvailable |
                  NoRemindMeLaterButton |
                  NoSkipThisVersionButton
};
};
};

#endif // QAPPIMAGE_UPDATE_CODES_HPP_INCLUDED
