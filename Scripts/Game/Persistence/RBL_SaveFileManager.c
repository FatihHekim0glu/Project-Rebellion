// ============================================================================
// PROJECT REBELLION - Save File Manager
// File I/O operations for save data
// ============================================================================

class RBL_SaveFileManager
{
	// Save file paths
	protected static const string SAVE_FOLDER = "$profile:Rebellion/saves/";
	protected static const string SAVE_EXTENSION = ".rblsave";
	protected static const string BACKUP_EXTENSION = ".rblbak";
	protected static const string QUICKSAVE_NAME = "quicksave";
	protected static const string AUTOSAVE_NAME = "autosave";
	
	protected static ref RBL_SaveFileManager s_Instance;
	
	// ========================================================================
	// SINGLETON
	// ========================================================================
	
	static RBL_SaveFileManager GetInstance()
	{
		if (!s_Instance)
			s_Instance = new RBL_SaveFileManager();
		return s_Instance;
	}
	
	void RBL_SaveFileManager()
	{
		// Ensure save directory exists
		EnsureSaveDirectory();
	}
	
	// ========================================================================
	// FILE OPERATIONS
	// ========================================================================
	
	// Write save data to file
	bool WriteToFile(string filename, string jsonContent)
	{
		if (filename.IsEmpty() || jsonContent.IsEmpty())
		{
			PrintFormat("[RBL_SaveFile] Write failed: empty filename or content");
			return false;
		}
		
		string filepath = GetSavePath(filename);
		
		// Create backup of existing file
		if (FileExists(filepath))
		{
			string backupPath = filepath + BACKUP_EXTENSION;
			CopyFile(filepath, backupPath);
		}
		
		// Write new content
		FileHandle file = FileIO.OpenFile(filepath, FileMode.WRITE);
		if (!file)
		{
			PrintFormat("[RBL_SaveFile] Failed to open file for writing: %1", filepath);
			return false;
		}
		
		file.Write(jsonContent);
		file.Close();
		
		PrintFormat("[RBL_SaveFile] Save written: %1 (%2 bytes)", filename, jsonContent.Length());
		return true;
	}
	
	// Read save data from file
	string ReadFromFile(string filename)
	{
		if (filename.IsEmpty())
		{
			PrintFormat("[RBL_SaveFile] Read failed: empty filename");
			return "";
		}
		
		string filepath = GetSavePath(filename);
		
		if (!FileExists(filepath))
		{
			PrintFormat("[RBL_SaveFile] File not found: %1", filepath);
			return "";
		}
		
		FileHandle file = FileIO.OpenFile(filepath, FileMode.READ);
		if (!file)
		{
			PrintFormat("[RBL_SaveFile] Failed to open file for reading: %1", filepath);
			return "";
		}
		
		string content = "";
		string line;
		while (file.ReadLine(line) >= 0)
		{
			content += line;
		}
		file.Close();
		
		PrintFormat("[RBL_SaveFile] Save loaded: %1 (%2 bytes)", filename, content.Length());
		return content;
	}
	
	// Delete save file
	bool DeleteFile(string filename)
	{
		string filepath = GetSavePath(filename);
		
		if (!FileExists(filepath))
			return true; // Already doesn't exist
		
		bool result = FileIO.DeleteFile(filepath);
		
		// Also delete backup if exists
		string backupPath = filepath + BACKUP_EXTENSION;
		if (FileExists(backupPath))
			FileIO.DeleteFile(backupPath);
		
		PrintFormat("[RBL_SaveFile] File deleted: %1", filename);
		return result;
	}
	
	// Check if file exists
	bool SaveExists(string filename)
	{
		return FileExists(GetSavePath(filename));
	}
	
	// ========================================================================
	// SLOT MANAGEMENT
	// ========================================================================
	
	// Get list of all save files
	array<string> GetSaveFileList()
	{
		array<string> saves = new array<string>();
		// File enumeration not supported in this environment.
		return saves;
	}
	
	// Get slot info for UI display
	RBL_SaveSlotInfo GetSlotInfo(int slotIndex)
	{
		RBL_SaveSlotInfo info = new RBL_SaveSlotInfo();
		info.m_iSlotIndex = slotIndex;
		
		string filename = GetSlotFilename(slotIndex);
		string filepath = GetSavePath(filename);
		
		if (!FileExists(filepath))
		{
			info.m_bIsEmpty = true;
			return info;
		}
		
		// Try to read and parse header only
		string content = ReadFromFile(filename);
		if (content.IsEmpty())
		{
			info.m_bIsCorrupted = true;
			return info;
		}
		
		RBL_SaveData saveData = RBL_SaveSerializer.DeserializeFromJson(content);
		if (!saveData || !saveData.IsValid())
		{
			info.m_bIsCorrupted = true;
			return info;
		}
		
		// Populate slot info
		info.m_bIsEmpty = false;
		info.m_sSlotName = "Slot " + slotIndex.ToString();
		info.m_sSaveTime = saveData.m_sSaveTime;
		info.m_sWorldName = saveData.m_sWorldName;
		info.m_fPlayTime = saveData.m_Campaign.m_fTotalPlayTime;
		info.m_iWarLevel = saveData.m_Campaign.m_iWarLevel;
		
		// Count zones
		int fiaZones = 0;
		for (int i = 0; i < saveData.m_aZones.Count(); i++)
		{
			if (saveData.m_aZones[i].m_iOwnerFaction == ERBLFactionKey.FIA)
				fiaZones++;
		}
		info.m_iZonesControlled = fiaZones;
		info.m_iTotalZones = saveData.m_aZones.Count();
		
		return info;
	}
	
	// Get all slot infos
	array<ref RBL_SaveSlotInfo> GetAllSlotInfos(int maxSlots)
	{
		array<ref RBL_SaveSlotInfo> infos = new array<ref RBL_SaveSlotInfo>();
		
		for (int i = 0; i < maxSlots; i++)
		{
			infos.Insert(GetSlotInfo(i));
		}
		
		return infos;
	}
	
	// ========================================================================
	// QUICKSAVE / AUTOSAVE
	// ========================================================================
	
	// Write quicksave
	bool WriteQuicksave(string jsonContent)
	{
		return WriteToFile(QUICKSAVE_NAME, jsonContent);
	}
	
	// Read quicksave
	string ReadQuicksave()
	{
		return ReadFromFile(QUICKSAVE_NAME);
	}
	
	// Check if quicksave exists
	bool QuicksaveExists()
	{
		return SaveExists(QUICKSAVE_NAME);
	}
	
	// Write autosave
	bool WriteAutosave(string jsonContent)
	{
		return WriteToFile(AUTOSAVE_NAME, jsonContent);
	}
	
	// Read autosave
	string ReadAutosave()
	{
		return ReadFromFile(AUTOSAVE_NAME);
	}
	
	// Check if autosave exists
	bool AutosaveExists()
	{
		return SaveExists(AUTOSAVE_NAME);
	}
	
	// ========================================================================
	// BACKUP AND RESTORE
	// ========================================================================
	
	// Restore from backup
	bool RestoreFromBackup(string filename)
	{
		string filepath = GetSavePath(filename);
		string backupPath = filepath + BACKUP_EXTENSION;
		
		if (!FileExists(backupPath))
		{
			PrintFormat("[RBL_SaveFile] No backup found for: %1", filename);
			return false;
		}
		
		// Copy backup over original
		bool result = CopyFile(backupPath, filepath);
		
		if (result)
			PrintFormat("[RBL_SaveFile] Restored from backup: %1", filename);
		
		return result;
	}
	
	// Create manual backup
	bool CreateBackup(string filename, string backupName)
	{
		string filepath = GetSavePath(filename);
		string backupPath = GetSavePath(backupName);
		
		if (!FileExists(filepath))
		{
			PrintFormat("[RBL_SaveFile] Cannot backup, file not found: %1", filename);
			return false;
		}
		
		return CopyFile(filepath, backupPath);
	}
	
	// ========================================================================
	// UTILITIES
	// ========================================================================
	
	// Ensure save directory exists
	protected void EnsureSaveDirectory()
	{
		FileIO.MakeDirectory(SAVE_FOLDER);
	}
	
	// Get full path for filename
	string GetSavePath(string filename)
	{
		// Add extension if not present
		if (!filename.Contains(SAVE_EXTENSION))
			filename = filename + SAVE_EXTENSION;
		
		return SAVE_FOLDER + filename;
	}
	
	// Get filename for slot index
	string GetSlotFilename(int slotIndex)
	{
		return "slot_" + slotIndex.ToString();
	}
	
	// Check if file exists at path
	protected bool FileExists(string path)
	{
		return FileIO.FileExists(path);
	}
	
	// Copy file
	protected bool CopyFile(string sourcePath, string destPath)
	{
		string content = "";
		
		// Read source
		FileHandle sourceFile = FileIO.OpenFile(sourcePath, FileMode.READ);
		if (!sourceFile)
			return false;
		
		string line;
		while (sourceFile.ReadLine(line) >= 0)
		{
			content += line + "\n";
		}
		sourceFile.Close();
		
		// Write destination
		FileHandle destFile = FileIO.OpenFile(destPath, FileMode.WRITE);
		if (!destFile)
			return false;
		
		destFile.Write(content);
		destFile.Close();
		
		return true;
	}
	
	// Get file size
	int GetFileSize(string filename)
	{
		string filepath = GetSavePath(filename);
		
		if (!FileExists(filepath))
			return 0;
		
		FileHandle file = FileIO.OpenFile(filepath, FileMode.READ);
		if (!file)
			return 0;
		
		int size = file.GetLength();
		file.Close();
		
		return size;
	}
	
	// Get file modification time
	string GetFileModifiedTime(string filename)
	{
		// This would need platform-specific implementation
		// For now return empty
		return "";
	}
	
	// ========================================================================
	// VALIDATION
	// ========================================================================
	
	// Validate save file integrity
	bool ValidateSaveFile(string filename)
	{
		string content = ReadFromFile(filename);
		if (content.IsEmpty())
			return false;
		
		RBL_SaveData saveData = RBL_SaveSerializer.DeserializeFromJson(content);
		if (!saveData)
			return false;
		
		return saveData.IsValid();
	}
	
	// Get save version
	int GetSaveVersion(string filename)
	{
		string content = ReadFromFile(filename);
		if (content.IsEmpty())
			return -1;
		
		RBL_SaveData saveData = RBL_SaveSerializer.DeserializeFromJson(content);
		if (!saveData)
			return -1;
		
		return saveData.m_iVersion;
	}
}

// ============================================================================
// FILE OPERATION RESULT - For async operations
// ============================================================================
class RBL_FileOperationResult
{
	bool m_bSuccess;
	string m_sMessage;
	string m_sFilename;
	int m_iBytes;
	
	void RBL_FileOperationResult(bool success, string message, string filename, int bytes)
	{
		m_bSuccess = success;
		m_sMessage = message;
		m_sFilename = filename;
		m_iBytes = bytes;
	}
}

