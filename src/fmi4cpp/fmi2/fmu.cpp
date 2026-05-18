
#include <fmi4cpp/fmi2/fmu.hpp>
#include <fmi4cpp/fmi2/xml/model_description_parser.hpp>
#include <fmi4cpp/mlog.hpp>
#include <fmi4cpp/tools/os_util.hpp>
#include <fmi4cpp/tools/simple_id.hpp>
#include <fmi4cpp/tools/unzipper.hpp>

#include <cassert>
#include <cstdlib>
#include <filesystem>
#include <stdexcept>
#include <strstream>
#include <utility>

using namespace fmi4cpp;
using namespace fmi4cpp::fmi2;

fmu::fmu(const std::filesystem::path& fmuPath, const std::string& aId)
{

    if (!exists(fmuPath)) {
        const auto err = "No such file '" + absolute(fmuPath).string() + "'!";
        MLOG_FATAL(err);
        throw std::runtime_error(err);
    }

    const std::string fmuName = fmuPath.stem().string();
    auto id = aId.empty() ? generate_simple_id(0) : aId;
    std::filesystem::path tmpPath(std::filesystem::temp_directory_path() /= std::filesystem::path("fmi4cpp_" + fmuName + "_" + id));

    if (exists(tmpPath)) {
        MLOG_INFO("removing stale directory");
        remove_all(tmpPath);
    }

    if (!create_directories(tmpPath)) {

        const auto err = "Failed to create temporary directory '" + tmpPath.string() + "' !";
        MLOG_FATAL(err);
        throw std::runtime_error(err);
    }

    MLOG_DEBUG("Created temporary directory '" << tmpPath.string());

    if (!unzip(fmuPath, tmpPath.string())) {
        const auto err = "Failed to extract FMU '" + absolute(fmuPath).string() + "'!";
        MLOG_FATAL(err);
        throw std::runtime_error(err);
    }

    resource_ = std::make_shared<fmu_resource>(tmpPath);
    modelDescription_ = std::move(parse_model_description(resource_->model_description_path()));

    // force linker to use specific .so

    auto fmuLib = std::filesystem::path(resource_->absolute_library_path(modelDescription_->as_cs_description()->model_identifier));
    MLOG_DEBUG("fmulib: " + fmuLib.filename().string() + "\n");
    std::filesystem::path libXml = fmuLib.parent_path() / "libDallaraXmlAccess.so";
    std::filesystem::path txtLib = fmuLib.parent_path() / "libDallaraTxtAccess.so";

    auto new_libXml = fmuLib.parent_path().string() + "/xml_" + aId + fmuLib.extension().string();
    auto new_txtLib = fmuLib.parent_path().string() + "/txt_" + aId + fmuLib.extension().string();

    assert(std::filesystem::exists(libXml));
    // rename
    std::filesystem::rename(libXml, new_libXml);
    std::filesystem::rename(txtLib, new_txtLib);
    MLOG_DEBUG(+"renamed xml: " + libXml.string() + " to " + new_libXml + "\n");

    // change linked dependencies using patchelf
    std::string cmd = "patchelf --replace-needed " + libXml.filename().string() + " " + new_libXml + " " + fmuLib.string();
    MLOG_DEBUG("cmd: " + cmd + "\n");

    std::system(cmd.c_str());
    cmd = "patchelf --replace-needed " + txtLib.filename().string() + " " + new_txtLib + " " + fmuLib.string();

    std::system(cmd.c_str());
    // throw std::runtime_error("STOP");
}

std::string fmu::get_model_description_xml() const
{
    return resource_->get_model_description_xml();
}

std::shared_ptr<const model_description> fmu::get_model_description() const
{
    return modelDescription_;
}

bool fmu::supports_me() const
{
    return modelDescription_->supports_me();
}

bool fmu::supports_cs() const
{
    return modelDescription_->supports_cs();
}

std::unique_ptr<cs_fmu> fmu::as_cs_fmu() const
{
    std::shared_ptr<const cs_model_description> cs = std::move(modelDescription_->as_cs_description());
    return std::make_unique<cs_fmu>(resource_, cs);
}

std::unique_ptr<me_fmu> fmu::as_me_fmu() const
{
    std::shared_ptr<const me_model_description> me = std::move(modelDescription_->as_me_description());
    return std::make_unique<me_fmu>(resource_, me);
}
