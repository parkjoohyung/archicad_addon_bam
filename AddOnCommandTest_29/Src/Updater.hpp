#ifndef UPDATER_HPP
#define UPDATER_HPP

namespace MyProjectNamespace {

    // Checks for updates against the GitHub repository.
    // If manual is true, it displays a message even if no update is found.
    void CheckForUpdates(bool manual = false);

    // Returns the current version string
    const char* GetCurrentVersion();

} // namespace MyProjectNamespace

#endif // UPDATER_HPP
