#include "cli_dataset.h"
#include <cstdio>

namespace ot_mini {

void Cli::UpdateNetDataMaxLength()
{
    size_t current = mLocalNetData.ToTlvBytes().size();
    if (current > mNetDataMaxLength) mNetDataMaxLength = current;
}

void Cli::ProcessNetData(const std::vector<std::string> &aArgs)
{
    if (aArgs.empty())
    {
        std::printf("Error: 'netdata help' yaz (netdata tek basina bir komut degil)\n");
        return;
    }

    const std::string &cmd = aArgs[0];

    if (cmd == "publish")
    {
        if (aArgs.size() > 1 && aArgs[1] == "dnssrp")
        {
            std::printf("Error: 'netdata publish dnssrp' desteklenmiyor - gercek bir Leader/TMF Network Data "
                         "servis kaydi gerektirir (ot_mini'de mesh yok)\n");
            return;
        }

        if (aArgs.size() < 3 || (aArgs[1] != "prefix" && aArgs[1] != "route"))
        {
            std::printf("Error: 'netdata publish prefix <prefix> [bayraklar] [pref]' ya da "
                         "'netdata publish route <prefix> [bayraklar] [pref]' yaz\n");
            return;
        }

        bool isRoute = (aArgs[1] == "route");

        NetDataEntry entry;
        entry.mIsRoute = isRoute;

        if (!ParseIp6Prefix(aArgs[2], entry.mPrefix, entry.mPrefixLength))
        {
            std::printf("Error: gecersiz prefix '%s' (ornek: fd00:1234:5678::/64)\n", aArgs[2].c_str());
            return;
        }

        std::string flags = aArgs.size() > 3 ? aArgs[3] : "";
        std::string pref  = aArgs.size() > 4 ? aArgs[4] : "";

        if (!ParseNetDataFlags(flags, isRoute, entry))
        {
            std::printf("Error: gecersiz bayrak harfi '%s' (%s icin gecerli harfler: %s)\n", flags.c_str(),
                         isRoute ? "route" : "prefix", isRoute ? "s n a" : "p a d c r o s n");
            return;
        }

        if (!ParsePreference(pref, entry.mPreference))
        {
            std::printf("Error: gecersiz tercih '%s' (high, med ya da low olmali)\n", pref.c_str());
            return;
        }

        mLocalNetData.PublishEntry(entry);
        UpdateNetDataMaxLength();
        std::printf("Done\n");
    }
    else if (cmd == "unpublish")
    {
        if (aArgs.size() > 1 && aArgs[1] == "dnssrp")
        {
            std::printf("Error: 'netdata unpublish dnssrp' desteklenmiyor - 'netdata publish dnssrp' hic "
                         "desteklenmedigi icin kaldirilacak bir sey yok\n");
            return;
        }

        if (aArgs.size() < 2)
        {
            std::printf("Error: 'netdata unpublish <prefix>' yaz\n");
            return;
        }

        Ip6Address prefix;
        uint8_t    length;

        if (!ParseIp6Prefix(aArgs[1], prefix, length))
        {
            std::printf("Error: gecersiz prefix '%s'\n", aArgs[1].c_str());
            return;
        }

        if (mLocalNetData.Unpublish(prefix, length) == 0)
        {
            std::printf("Error: bu prefix yayinlanmiyor\n");
            return;
        }

        UpdateNetDataMaxLength();
        std::printf("Done\n");
    }
    else if (cmd == "show")
    {
        bool hexMode = false;
        bool isLocal = false;

        for (size_t i = 1; i < aArgs.size(); i++)
        {
            if (aArgs[i] == "-x") hexMode = true;
            else if (aArgs[i] == "local") isLocal = true;
            else
            {
                // Gercek koddaki opsiyonel <rloc16> filtresi: bizde gercek bir
                // Leader/partition netdata'si olmadigindan anlamsiz.
                std::printf("Error: 'netdata show' icin rloc16 filtresi ya da Leader'dan alinan netdata "
                             "desteklenmiyor - sadece 'netdata show local [-x]' calisir\n");
                return;
            }
        }

        if (!isLocal)
        {
            std::printf("Error: duz 'netdata show' Leader'dan gelen partition netdata'sini gosterir; "
                         "ot_mini'de gercek bir Leader yok. 'netdata show local [-x]' kullan\n");
            return;
        }

        if (hexMode)
        {
            std::printf("%s\n", mLocalNetData.ToHexTlvs().c_str());
        }
        else
        {
            mLocalNetData.Print();
        }

        std::printf("Done\n");
    }
    else if (cmd == "length")
    {
        std::printf("%zu\n", mLocalNetData.ToTlvBytes().size());
        std::printf("Done\n");
    }
    else if (cmd == "maxlength")
    {
        if (aArgs.size() > 1 && aArgs[1] == "reset")
        {
            mNetDataMaxLength = 0;
            UpdateNetDataMaxLength();
            std::printf("Done\n");
        }
        else
        {
            std::printf("%zu\n", mNetDataMaxLength);
            std::printf("Done\n");
        }
    }
    else if (cmd == "help")
    {
        std::printf("netdata komutlari:\n");
        std::printf("  help length maxlength publish unpublish show\n");
        std::printf("Done\n");
    }
    else if (cmd == "register" || cmd == "steeringdata")
    {
        std::printf(
            "Error: 'netdata %s' desteklenmiyor - gercek bir Leader/commissioning durumu gerektirir "
            "(ot_mini'de mesh yok)\n",
            cmd.c_str());
    }
    else
    {
        std::printf("Error: bilinmeyen 'netdata' komutu '%s' (yardim: netdata help)\n", cmd.c_str());
    }
}

} // namespace ot_mini
