#ifndef LICENSING_HPP
#define LICENSING_HPP

#include "ACAPinc.h"

namespace MyProjectNamespace {

    // Computes the license hash
    GS::UniString ComputeLicenseHash(const GS::UniString& email, const GS::UniString& key);

    // Checks online if the license hash is valid
    bool CheckLicenseOnline(const GS::UniString& email, const GS::UniString& key);

    // Checks the local registry to see if the user is already licensed
    bool IsAlreadyLicensed();

    // Saves the license into the local registry
    bool SaveLicenseLocal(const GS::UniString& email, const GS::UniString& key);

} // namespace MyProjectNamespace

#endif // LICENSING_HPP
