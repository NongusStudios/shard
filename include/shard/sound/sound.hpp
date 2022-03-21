#pragma once

#include <string>
#include <array>
#include <queue>

#include <miniaudio.h>

namespace shard{
    class Sound{
        public:
            static constexpr uint32_t MAX_SOUNDS = 1000;

            enum class Result{
                SUCCESS = 0,
                FAILED  = 1
            };

            Sound();
            ~Sound();

            uint32_t load(const std::string& filePath);
            void     unload(uint32_t sound);

            bool isPlaying(uint32_t sound);
            bool isLooping(uint32_t sound);
            void setLooping(uint32_t sound, bool isLooping);
            void setVolume(uint32_t sound, float volume);
            float getVolume(uint32_t sound);
            float getVolume();
            Result setVolume(float volume);
            Result play(const std::string& filePath);
            Result play(uint32_t sound);
            Result start();
            Result stop();
        private:
            Result convertMaResultToResult(ma_result result){
                if(result == MA_SUCCESS) return Result::SUCCESS;
                else return Result::FAILED;
            }

            ma_engine soundEngine = {};
            std::array<ma_sound, MAX_SOUNDS> sounds      = {};
            std::array<bool, MAX_SOUNDS>     soundsAlloc = {false};
            std::queue<uint32_t> availableSoundNames;
            float engineVolume = 1.0f;
    };
} // namespace shard
