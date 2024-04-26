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

#include <pag/file.h>
#include <pag/pag.h>
#include <sys/stat.h>

#include <iostream>
#include <vector>
#include <string>
#include <cstdio>
#include <cstdlib>

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
#include <libavutil/opt.h>
}

int64_t GetTimer() {
    static auto START_TIME = std::chrono::high_resolution_clock::now();
    auto now = std::chrono::high_resolution_clock::now();
    auto ns = std::chrono::duration_cast<std::chrono::nanoseconds>(now - START_TIME);
    return static_cast<int64_t>(ns.count() * 1e-3);
}

std::shared_ptr<pag::PAGFile> ReplaceImageOrText() {
//    auto pagFile = pag::PAGFile::Load("../../assets/alpha_matte.pag");
  auto pagFile = pag::PAGFile::Load("../resources/pag/hou.pag");
    if (pagFile == nullptr) {
        return nullptr;
    }
    const int pathSize = 3;
    std::string picPathList[pathSize] {"../resources/yuzhou.jpg", "../resources/girl-2.jpg", "../resources/girl-1.jpg"};
    for (int i = 0; i < pagFile->numImages(); i++) {
        auto pagImage = pag::PAGImage::FromPath(picPathList[i % pathSize]);
        pagFile->replaceImage(i, pagImage);
    }

    for (int i = 0; i < pagFile->numTexts(); i++) {
        auto textDocumentHandle = pagFile->getTextData(i);
        textDocumentHandle->text = "hah哈 哈哈哈哈👌";
        // Use special font
        auto pagFont = pag::PAGFont::RegisterFont("../../resources/font/NotoSansSC-Regular.otf", 0);
        textDocumentHandle->fontFamily = pagFont.fontFamily;
        textDocumentHandle->fontStyle = pagFont.fontStyle;
        pagFile->replaceText(i, textDocumentHandle);
    }

    return pagFile;
}

int64_t TimeToFrame(int64_t time, float frameRate) {
    return static_cast<int64_t>(floor(time * frameRate / 1000000ll));
}

void BmpWrite(unsigned char *image, int imageWidth, int imageHeight, const char *filename) {
    unsigned char header[54] = {0x42, 0x4d, 0, 0, 0, 0, 0, 0, 0, 0, 54, 0, 0, 0, 40, 0, 0, 0,
                                0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 32, 0, 0, 0, 0, 0, 0, 0,
                                0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

    int64_t file_size = static_cast<int64_t>(imageWidth) * static_cast<int64_t>(imageHeight) * 4 + 54;
    header[2] = static_cast<unsigned char>(file_size & 0x000000ff);
    header[3] = (file_size >> 8) & 0x000000ff;
    header[4] = (file_size >> 16) & 0x000000ff;
    header[5] = (file_size >> 24) & 0x000000ff;

    int64_t width = imageWidth;
    header[18] = width & 0x000000ff;
    header[19] = (width >> 8) & 0x000000ff;
    header[20] = (width >> 16) & 0x000000ff;
    header[21] = (width >> 24) & 0x000000ff;

    int64_t height = -imageHeight;
    header[22] = height & 0x000000ff;
    header[23] = (height >> 8) & 0x000000ff;
    header[24] = (height >> 16) & 0x000000ff;
    header[25] = (height >> 24) & 0x000000ff;

    char fname_bmp[128];
    snprintf(fname_bmp, 128, "%s.bmp", filename);

    FILE *fp;
    if (!(fp = fopen(fname_bmp, "wb"))) {
        return;
    }

    fwrite(header, sizeof(unsigned char), 54, fp);
    fwrite(image, sizeof(unsigned char), (size_t) (int64_t) imageWidth * imageHeight * 4, fp);
    fclose(fp);
}

void BmpWriteBGR(unsigned char *image, int imageWidth, int imageHeight, const char *filename) {
    unsigned char header[54] = {0x42, 0x4d, 0, 0, 0, 0, 0, 0, 0, 0, 54, 0, 0, 0, 40, 0, 0, 0,
                                0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 24, 0, 0, 0, 0, 0, 0, 0,
                                0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

    int64_t file_size = static_cast<int64_t>(imageWidth) * static_cast<int64_t>(imageHeight) * 3 + 54;
    header[2] = static_cast<unsigned char>(file_size & 0x000000ff);
    header[3] = (file_size >> 8) & 0x000000ff;
    header[4] = (file_size >> 16) & 0x000000ff;
    header[5] = (file_size >> 24) & 0x000000ff;

    int64_t width = imageWidth;
    header[18] = width & 0x000000ff;
    header[19] = (width >> 8) & 0x000000ff;
    header[20] = (width >> 16) & 0x000000ff;
    header[21] = (width >> 24) & 0x000000ff;

    int64_t height = -imageHeight;
    header[22] = height & 0x000000ff;
    header[23] = (height >> 8) & 0x000000ff;
    header[24] = (height >> 16) & 0x000000ff;
    header[25] = (height >> 24) & 0x000000ff;

    char fname_bmp[128];
    snprintf(fname_bmp, 128, "%s.bmp", filename);

    FILE *fp;
    if (!(fp = fopen(fname_bmp, "wb"))) {
        return;
    }

    fwrite(header, sizeof(unsigned char), 54, fp);
    fwrite(image, sizeof(unsigned char), (size_t) (int64_t) imageWidth * imageHeight * 3, fp);
    fclose(fp);
}

int main() {
    if (system("rm -rf output") != 0) {
        return 1;
    }

    mkdir("output", 0777);
    auto startTime = GetTimer();
    // Register fallback fonts. It should be called only once when the application is being initialized.
    std::vector<std::string> fallbackFontPaths = {};
    fallbackFontPaths.emplace_back("../../resources/font/NotoSerifSC-Regular.otf");
    fallbackFontPaths.emplace_back("../../resources/font/NotoColorEmoji.ttf");
    std::vector<int> ttcIndices(fallbackFontPaths.size());
    pag::PAGFont::SetFallbackFontPaths(fallbackFontPaths, ttcIndices);

    auto pagFile = ReplaceImageOrText();
    if (pagFile == nullptr) {
        printf("---pagFile is nullptr!!!\n");
        return -1;
    }

    auto pagSurface = pag::PAGSurface::MakeOffscreen(pagFile->width(), pagFile->height());
    if (pagSurface == nullptr) {
        printf("---pagSurface is nullptr!!!\n");
        return -1;
    }
    auto pagPlayer = new pag::PAGPlayer();
    pagPlayer->setSurface(pagSurface);
    pagPlayer->setComposition(pagFile);
    auto totalFrames = TimeToFrame(pagFile->duration(), pagFile->frameRate());
    auto currentFrame = 0;
    auto frameRate = pagFile->frameRate();

    printf("------totalFrames:%d, frameRate:%f \n", totalFrames, frameRate);

    int bytesLength = pagFile->width() * pagFile->height() * 4;

    while (currentFrame <= totalFrames) {
        pagPlayer->setProgress(currentFrame * 1.0 / totalFrames);
        auto status = pagPlayer->flush();

        printf("---currentFrame:%d, flushStatus:%d \n", currentFrame, status);

        auto data = new uint8_t[bytesLength];

        pagSurface->readPixels(pag::ColorType::BGRA_8888, pag::AlphaType::Premultiplied, data,
                               pagFile->width() * 4);

        auto width = pagFile->width();
        auto height = pagFile->height();
        auto alphaLength = width * 2 * height * 3;
        auto alphaData = new uint8_t[alphaLength];
        for (int j = 0; j < height; j++) {
            for (int i = 0; i < width; i++) {
                auto alphaStart = width * (2 * j) * 3 + i * 3;
                alphaData[alphaStart] = alphaData[alphaStart + 1] = alphaData[alphaStart + 2] = data[width * j * 4 + (i * 4) + 3];
//                alphaData[alphaStart + 3] = 255;
                auto rgbStart = width * (2 * j + 1) * 3 + i * 3;
                alphaData[rgbStart] = data[width * j * 4 + (i * 4)];
                alphaData[rgbStart + 1] = data[width * j * 4 + (i * 4) + 1];
                alphaData[rgbStart + 2] = data[width * j * 4 + (i * 4) + 2];
//                alphaData[rgbStart + 3] = 255;
            }
        }

        std::string imageName = std::to_string(currentFrame);
        BmpWriteBGR(alphaData, width * 2, height, ("output/" + imageName).c_str());

        delete[] data;
        delete[] alphaData;

        currentFrame++;
    }

    delete pagPlayer;

    printf("----timeCost--:%ld \n", static_cast<long>(GetTimer() - startTime));

    // 通过ffmpeg合成mp4
    char ffmpegCmd[256];
    system("which ffmpeg");
    sprintf(ffmpegCmd,
            "ffmpeg -framerate %d -i 'output/%%d.bmp' -crf 20 -b:v 0 -preset fast -vcodec libx264 -pix_fmt yuv420p -vf 'pad=ceil(iw/2)*2:ceil(ih/2)*2' -r 30 output/out.mp4",
            (int) frameRate);
    printf("ffmpegCmd: %s\n", ffmpegCmd);
    auto result = system(ffmpegCmd);

    printf("----totalTimeCost--:%ld \n", static_cast<long>(GetTimer() - startTime));

 // Initialize FFmpeg
    av_register_all();
    avcodec_register_all();


    // Create the output format context
    AVFormatContext *ofmt_ctx = nullptr;
    avformat_alloc_output_context2(&ofmt_ctx, nullptr, "mp4", "test.mp4");
    if (!ofmt_ctx) {
        std::cerr << "Could not create output format context" << std::endl;
        return 1;
    }

    return result;
}
