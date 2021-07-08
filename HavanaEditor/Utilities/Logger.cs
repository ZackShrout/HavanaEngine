using System;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.IO;
using System.Runtime.CompilerServices;
using System.Text;
using System.Windows;
using System.Windows.Data;

namespace HavanaEditor.Utilities
{
    /// <summary>
    /// Message types.
    /// </summary>
    enum MessageType
    {
        Info = 0x01,
        Warning = 0x02,
        Error = 0x04
    }

    /// <summary>
    /// Data that defines the log message.
    /// </summary>
    class LogMessage
    {
        // PROPERTIES
        public DateTime Time { get; }
        public MessageType MessageType { get; }
        public string Message { get; }
        public string File { get; }
        public string Caller { get; }
        public int Line { get; }
        public string MetaData => $"{File}: {Caller} ({Line})";

        // PUBLIC
        public LogMessage(MessageType type, string message, string file, string caller, int line)
        {
            Time = DateTime.Now;
            MessageType = type;
            Message = message;
            File = Path.GetFileName(file);
            Caller = caller;
            Line = line;
        }
    }

    /// <summary>
    /// Class which handles logging messages to the editor window.
    /// </summary>
    static class Logger
    {
        // STATE
        private static int messageFilter = (int)(MessageType.Info | MessageType.Warning | MessageType.Error);
        private static readonly ObservableCollection<LogMessage> messages = new ObservableCollection<LogMessage>();

        // PROPERTIES
        public static ReadOnlyObservableCollection<LogMessage> Messages { get; } = new ReadOnlyObservableCollection<LogMessage>(messages);
        public static CollectionViewSource FilteredMessages { get; } = new CollectionViewSource() { Source = messages };

        // PUBLIC
        static Logger()
        {
            FilteredMessages.Filter += (s, e) =>
            {
                int type = (int)(e.Item as LogMessage).MessageType;
                e.Accepted = (type & messageFilter) != 0;
            };
        }

        /// <summary>
        /// Logs a message.
        /// </summary>
        /// <param name="type">Message type.</param>
        /// <param name="message">Message content.</param>
        /// <param name="file">Path of the file the log is called from.</param>
        /// <param name="caller">Name of the caller the log is being called from.</param>
        /// <param name="line">Line of code the log is being called from.</param>
        public static async void Log(MessageType type, string message, [CallerFilePath]string file = "", [CallerMemberName]string caller = "",
            [CallerLineNumber]int line = 0)
        {
            await Application.Current.Dispatcher.BeginInvoke(new Action(() =>
            {
                messages.Add(new LogMessage(type, message, file, caller, line));
            }));
        }

        /// <summary>
        /// Clear messages collection.
        /// </summary>
        public static async void Clear()
        {
            await Application.Current.Dispatcher.BeginInvoke(new Action(() =>
            {
                messages.Clear();
            }));
        }

        /// <summary>
        /// Filters messages by types.
        /// </summary>
        /// <param name="mask">Mask made by logically oring MessageTypes together.</param>
        public static void SetFilter(int mask)
        {
            messageFilter = mask;
            FilteredMessages.View.Refresh();
        }
    }
}
