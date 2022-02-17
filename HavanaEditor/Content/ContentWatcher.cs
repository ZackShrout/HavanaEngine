﻿using HavanaEditor.Utilities;
using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.IO;
using System.Linq;
using System.Text;
using System.Windows;

namespace HavanaEditor.Content
{
    public class ContentModifiedEventArgs : EventArgs
    {
        // PROPERTIES
        public string FullPath { get; }

        public ContentModifiedEventArgs(string path)
        {
            FullPath = path;
        }
    }

    static class ContentWatcher
    {
        // STATE
        private static readonly DelayEventTimer _refreshTimer = new DelayEventTimer(TimeSpan.FromMilliseconds(250));
        private static readonly FileSystemWatcher _contentWatcher = new FileSystemWatcher()
        {
            IncludeSubdirectories = true,
            Filter = "",
            NotifyFilter = NotifyFilters.CreationTime |
                           NotifyFilters.DirectoryName |
                           NotifyFilters.FileName |
                           NotifyFilters.LastWrite
        };
        private static int _fileWatcherEnableCounter = 0; // filewatcher is only enabled when this counter is 0

        // EVENTS
        public static event EventHandler<ContentModifiedEventArgs> ContentModified;

        // PUBLIC
        static ContentWatcher()
        {
            _contentWatcher.Changed += OnContentModified;
            _contentWatcher.Created += OnContentModified;
            _contentWatcher.Deleted += OnContentModified;
            _contentWatcher.Renamed += OnContentModified;

            _refreshTimer.Triggered += Refresh;
        }

        public static void EnableFileWatcher(bool isEnabled)
        {
            if (_fileWatcherEnableCounter > 0 && isEnabled)
            {
                --_fileWatcherEnableCounter;
            }
            else if (!isEnabled)
            {
                ++_fileWatcherEnableCounter;
            }
        }

        public static void Reset(string contentFolder, string projectPath)
        {
            _contentWatcher.EnableRaisingEvents = false;

            ContentInfoCache.Reset(projectPath);

            if (!string.IsNullOrEmpty(contentFolder))
            {
                Debug.Assert(Directory.Exists(contentFolder));
                _contentWatcher.Path = contentFolder;
                _contentWatcher.EnableRaisingEvents = true;
                AssetRegistry.Reset(contentFolder);
            }
        }

        // PRIVATE
        private static async void OnContentModified(object sender, FileSystemEventArgs e)
        {
            await Application.Current.Dispatcher.BeginInvoke(new Action(() => _refreshTimer.Trigger(e)));
        }
        
        private static void Refresh(object sender, DelayEventTimerArgs e)
        {
            if (_fileWatcherEnableCounter > 0)
            {
                e.RepeatEvent = true;
                return;
            }

            e.Data
                .Cast<FileSystemEventArgs>()
                .GroupBy(x => x.FullPath)
                .Select(x => x.First())
                .ToList().ForEach(x => ContentModified?.Invoke(null, new ContentModifiedEventArgs(x.FullPath)));
        }
    }    
}
