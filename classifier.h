#pragma once
#include <string>
#include <vector>
#include <onnxruntime_cxx_api.h>
#include <cpu_provider_factory.h>
#include "fileInterface.h"
#include "fileUnit/TextSegmenter.h"
#include "fileUnit/vocabulary_loader.h"

struct ClassificationResult {
    std::string predicted_class;
    float confidence;
    std::vector<float> probabilities;
    std::vector<std::string> categories;
    bool success;
    std::string error_message;
    size_t word_count;
    
    ClassificationResult() : success(false), confidence(0.0f), word_count(0) {}
};

static const int MAX_TEXT_WORDS = 1000;
static const int MAX_FILENAME_WORDS = 32;

static const std::string categories[] = {
    "语文",
    "数学",
    "英语",
    "物理",
    "化学",
    "生物",
    "班会"
};

static ClassificationResult classify_file(
        const std::string& filepath,
        TextProcessor::TextSegmenter& text_segmenter,
        QString filename_vocab_file_path ,
        QString text_vocab_file_path ,
        const std::string& model_path
    ) 
{
    
    ClassificationResult result;
    std::string text_content = processfile(filepath);
    text_content = TextProcessor::cleanText(text_content);

    std::vector<std::string> words = text_segmenter.cutWords(text_content);
    result.word_count = words.size();

    // 加载词汇表
    VocabularyLoader filename_vocab;
    filename_vocab.loadFromFile(filename_vocab_file_path);
        

    VocabularyLoader text_vocab;
    text_vocab.loadFromFile(text_vocab_file_path);

    //转换

    // 将文本分词结果转换为索引
    std::vector<int32_t> text_indices(MAX_TEXT_WORDS, 0);
    for (size_t i = 0; i < words.size() && i < MAX_TEXT_WORDS; ++i) {
        int rank = text_vocab.getRank(QString::fromStdString(words[i].c_str())) >= 20000 ? 0 : text_vocab.getRank(QString::fromStdString(words[i].c_str()));
        text_indices[i] = (rank > 0) ? rank : 0; // 未找到的词用0表示
    }

    // 文件名
    std::vector<int32_t> filename_indices(MAX_FILENAME_WORDS, 0);
    std::filesystem::path p(filepath);
    std::string filename = p.stem().string();

    // 将文件名分词结果转换为索引
    TextProcessor::FilenameSegmenter filename_segmenter(&text_segmenter);
    std::vector<std::string> filename_words = filename_segmenter.cutFilename(filename);
    for (size_t i = 0; i < filename_words.size() && i < MAX_FILENAME_WORDS; ++i) {
        int rank = filename_vocab.getRank(QString::fromStdString(filename_words[i].c_str())) >= 20000 ? 0 : filename_vocab.getRank(QString::fromStdString(filename_words[i].c_str()));
        filename_indices[i] = (rank > 0) ? rank : 0; // 未找到的词用0表示
    }

    //推理
    Ort::Env env(ORT_LOGGING_LEVEL_WARNING, "text_classifier");
    Ort::SessionOptions session_options;
    session_options.SetIntraOpNumThreads(1);
    session_options.SetGraphOptimizationLevel(GraphOptimizationLevel::ORT_ENABLE_ALL);
            
    OrtSessionOptionsAppendExecutionProvider_CPU(session_options, 0);
            
    Ort::Session session(env, model_path.c_str(), session_options);
            
    // 输入输出名称
    std::vector<const char*> input_names = {"text_input", "filename_input"};
    std::vector<const char*> output_names = {"output"};
            
    // 创建内存信息
    Ort::MemoryInfo memory_info = Ort::MemoryInfo::CreateCpu(
        OrtArenaAllocator, OrtMemTypeDefault);
            
    // 准备输入张量
    std::vector<int64_t> text_shape = {1, MAX_TEXT_WORDS};
    std::vector<int64_t> filename_shape = {1, MAX_FILENAME_WORDS};

    Ort::Value text_input_tensor = Ort::Value::CreateTensor<int32_t>(
        memory_info, text_indices.data(), text_indices.size(), text_shape.data(), text_shape.size());

    Ort::Value filename_input_tensor = Ort::Value::CreateTensor<int32_t>(
        memory_info, filename_indices.data(), filename_indices.size(), filename_shape.data(), filename_shape.size());
    
    // 运行推理
    std::vector<Ort::Value> input_tensors;
    input_tensors.push_back(std::move(text_input_tensor));
    input_tensors.push_back(std::move(filename_input_tensor));

    auto output_tensors = session.Run(Ort::RunOptions{nullptr}, input_names.data(), input_tensors.data(), input_tensors.size(), output_names.data(), output_names.size());

    // 获取输出张量
    auto& output_tensor = output_tensors[0];
    float* output_data = output_tensor.GetTensorMutableData<float>();
    size_t output_size = output_tensor.GetTensorTypeAndShapeInfo().GetElementCount();

    result.probabilities.assign(output_data, output_data + output_size);
    result.categories = std::vector<std::string>(std::begin(categories), std::end(categories));

    int pred_idx = 0;
    float max_prob = result.probabilities[0];
    for (size_t i = 1; i < result.probabilities.size(); ++i) {
        if (result.probabilities[i] > max_prob) {
            max_prob = result.probabilities[i];
            pred_idx = i;
        }
    }

    result.predicted_class = categories[pred_idx];
    result.confidence = max_prob;
    result.success = true;

    return result;
}