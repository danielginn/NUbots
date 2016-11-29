#include "SpinnakerCamera.h"

#include "message/support/Configuration.h"

namespace module {
namespace input {

    using message::support::Configuration;

    SpinnakerCamera::SpinnakerCamera(std::unique_ptr<NUClear::Environment> environment)
    : Reactor(std::move(environment)), system(Spinnaker::System::GetInstance()), cameras() {

        on<Configuration>("Cameras/").then([this] (const Configuration& config)
        {
            Spinnaker::CameraList camList = system->GetCameras();

            if (camList.GetSize() < 1)
            {
                return;
            }

            std::string serialNumber = config["device"]["serial"].as<std::string>();

            // See if we already have this camera
            auto camera = cameras.find(serialNumber);

            if (camera == cameras.end())
            {
                // Get camera from system enumerated list.
                auto newCamera = camList.GetBySerial(serialNumber);

                // Ensure we found the camera.
                if (newCamera)
                {
                    // Initlise the camera.
                    newCamera->Init();

                    // Add camera to list.
                    struct ImageEvent imageEvent(serialNumber, std::move(newCamera), *this);
                    camera = cameras.insert(std::make_pair(serialNumber, imageEvent)).first;
                }

                else
                {
                    log("Failed to find camera with serial number: ", serialNumber);
                    return;
                }
            }

            // Get device node map.
            auto& nodeMap = camera->second.camera->GetNodeMap();

            // Set the pixel format.
            Spinnaker::GenApi::CEnumerationPtr ptrPixelFormat = nodeMap.GetNode("PixelFormat");

            if (IsAvailable(ptrPixelFormat) && IsWritable(ptrPixelFormat))
            {
                // Retrieve the desired entry node from the enumeration node
                std::string format = config["format"]["pixel"].as<std::string>();
                Spinnaker::GenApi::CEnumEntryPtr newPixelFormat = ptrPixelFormat->GetEntryByName(format.c_str());

                if (IsAvailable(newPixelFormat) && IsReadable(newPixelFormat))
                {
                    newPixelFormat->SetIntValue(newPixelFormat->GetValue());
                }

                else
                {
                    log("Failed to set pixel format to ", format, " for camera ", camera->first);
                }
            }

            else
            {
                log("Failed to retrieve pixel format for camera ", camera->first);
            }

            // Set the width and height of the image.
            Spinnaker::GenApi::CIntegerPtr ptrWidth = nodeMap.GetNode("Width");

            if (IsAvailable(ptrWidth) && IsWritable(ptrWidth))
            {
                int64_t width = config["format"]["width"].as<int>();

                // Ensure the width is a multiple of the increment.
                if ((width % ptrWidth->GetInc()) != 0)
                {
                    width = std::min(ptrWidth->GetMax(), std::max(ptrWidth->GetMin(), width - (width % ptrWidth->GetInc())));
                }

                ptrWidth->SetValue(width);
            }

            else
            {
                log("Failed to retrieve image width for camera ", camera->first);
            }

            Spinnaker::GenApi::CIntegerPtr ptrHeight = nodeMap.GetNode("Height");

            if (IsAvailable(ptrHeight) && IsWritable(ptrHeight))
            {
                int64_t height = config["format"]["height"].as<int>();

                // Ensure the height is a multiple of the increment.
                if ((height % ptrHeight->GetInc()) != 0)
                {
                    height = std::min(ptrHeight->GetMax(), std::max(ptrHeight->GetMin(), height - (height % ptrHeight->GetInc())));
                }

                ptrHeight->SetValue(height);
            }

            else
            {
                log("Failed to retrieve image height for camera ", camera->first);
            }

            // Set acquisition mode to continuous
            Spinnaker::GenApi::CEnumerationPtr ptrAcquisitionMode = nodeMap.GetNode("AcquisitionMode");

            if (!IsAvailable(ptrAcquisitionMode) || !IsWritable(ptrAcquisitionMode))
            {
                log("Failed to retrieve acquisition mode for camera ", camera->first);
                return;
            }

            Spinnaker::GenApi::CEnumEntryPtr ptrAcquisitionModeContinuous = ptrAcquisitionMode->GetEntryByName("Continuous");
            
            if (!IsAvailable(ptrAcquisitionModeContinuous) || !IsReadable(ptrAcquisitionModeContinuous))
            {
                log("Failed to retrieve continuous acquisition mode entry for camera ", camera->first);
                return;
            }

            ptrAcquisitionMode->SetIntValue(ptrAcquisitionModeContinuous->GetValue());

            // Setup the event handler for image acquisition.
            camera->second.camera->RegisterEvent(camera->second);

            // Begin acquisition.
            camera->second.camera->BeginAcquisition();
        });
    }
}
}
