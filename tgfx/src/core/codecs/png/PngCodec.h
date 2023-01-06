/////////////////////////////////////////////////////////////////////////////////////////////////
//
//  Tencent is pleased to support the open source community by making libpag available.
//
//  Copyright (C) 2021 THL A29 Limited, a Tencent company. All rights reserved.
//
//  Licensed under the Apache License, Version 2.0 (the "License"); you may not use this file
//  except in compliance with the License. You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
//  unless required by applicable law or agreed to in writing, software distributed under the
//  license is distributed on an "as is" basis, without warranties or conditions of any kind,
//  either express or implied. see the license for the specific language governing permissions
//  and limitations under the license.
//
/////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "tgfx/core/ImageCodec.h"

namespace tgfx {
class PngCodec : public ImageCodec {
 public:
  static std::shared_ptr<ImageCodec> MakeFrom(const std::string& filePath);
  static std::shared_ptr<ImageCodec> MakeFrom(std::shared_ptr<Data> imageBytes);
  static bool IsPng(const std::shared_ptr<Data>& data);

#ifdef TGFX_USE_PNG_ENCODE
  static std::shared_ptr<Data> Encode(const ImageInfo& info, const void* pixels,
                                      EncodedFormat format, int quality);
#endif

 protected:
  bool readPixels(const ImageInfo& dstInfo, void* dstPixels) const override;

 private:
  static std::shared_ptr<ImageCodec> MakeFromData(const std::string& filePath,
                                                  std::shared_ptr<Data> byteData);

  PngCodec(int width, int height, Orientation orientation, std::string filePath,
           std::shared_ptr<Data> fileData)
      : ImageCodec(width, height, orientation),
        fileData(std::move(fileData)),
        filePath(std::move(filePath)) {
  }

  std::shared_ptr<Data> fileData;
  std::string filePath;
};
}  // namespace tgfx