using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using System.Runtime.InteropServices;
using UnityEngine.UI;
using System;

public class VisSyncClient : MonoBehaviour
{
    [DllImport("VLUnityConnector")]
    private static extern int createClientAPI(string key, int port);
    [DllImport("VLUnityConnector")]
    private static extern IntPtr GetCreateAPIFunc();
    [DllImport("VLUnityConnector")]
    private static extern bool isAPIReady(int api);

    [DllImport("VLUnityConnector")]
    private static extern IntPtr getMessageQueue(int api, string name);
    [DllImport("VLUnityConnector")]
    private static extern void waitForMessage(IntPtr msgQueue);
    [DllImport("VLUnityConnector")]
    private static extern void sendMessage(IntPtr msgQueue);
    [DllImport("VLUnityConnector")]
    private static extern int queueRecieveInt(IntPtr msgQueue);
    [DllImport("VLUnityConnector")]
    private static extern void queueSendInt(IntPtr msgQueue, int val);
    [DllImport("VLUnityConnector")]
    private static extern void queueRecieveFloatArray(IntPtr msgQueue, [In, Out] IntPtr arr, int size);
    [DllImport("VLUnityConnector")]
    private static extern int queueReceiveString(IntPtr msgQueue, [In, Out] IntPtr data);

    public float[] getFloatArray(IntPtr msgQueue, int size)
    {
        float[] arr = new float[size];
        GCHandle h = GCHandle.Alloc(arr, GCHandleType.Pinned);
        queueRecieveFloatArray(msgQueue, h.AddrOfPinnedObject(), size);
        h.Free();
        return arr;
    }

    public Matrix4x4 getMatrix4x4(IntPtr msgQueue)
    {
        float[] arr = getFloatArray(msgQueue, 16);
        Matrix4x4 m = new Matrix4x4();
        for (int f = 0; f < arr.Length; f++)
        {
            m[f] = arr[f];
        }
        return m;
    }

    public string receiveString(IntPtr msgQueue)
    {
        byte[] data = new byte[50];
        GCHandle h = GCHandle.Alloc(data, GCHandleType.Pinned);
        int len = queueReceiveString(msgQueue, h.AddrOfPinnedObject());
        h.Free();
        return System.Text.Encoding.UTF8.GetString(data).Substring(0, len);
    }

    public string address = "127.0.0.1";
    public int port = 3457;
    public string clientName = "client";
    private int api;
    private Dictionary<int, GameObject> views = new Dictionary<int, GameObject>();
    private IntPtr startFrame;
    private IntPtr finishFrame;

    public int GetAPI()
    {
        return api;
    }

    public bool IsReady()
    {
        return isAPIReady(api);
    }

    void Start()
    {
        api = createClientAPI(address, port);
        GL.IssuePluginEvent(GetCreateAPIFunc(), api);
        startFrame = getMessageQueue(api, clientName + "-start");
        finishFrame = getMessageQueue(api, clientName + "-finish");
    }

    void Update()
    {
        waitForMessage(startFrame);
        Debug.Log("recieved frame");
        int visSyncId = queueRecieveInt(startFrame);
        Debug.Log("Id: " + visSyncId);
        int frameVal = queueRecieveInt(startFrame);
        Matrix4x4 projMatrix = getMatrix4x4(startFrame);
        Matrix4x4 viewMatrix = getMatrix4x4(startFrame);
        Matrix4x4 modelMatrix = getMatrix4x4(startFrame);

        GameObject view;
        if (!views.TryGetValue(visSyncId, out view))
        {
            sendMessage(finishFrame);
            queueSendInt(finishFrame, 1);

            waitForMessage(startFrame);
            int viewsPerFrame = queueRecieveInt(startFrame);
            string texturePrefix = receiveString(startFrame);
            Debug.Log("views per frame: " + viewsPerFrame);
            Debug.Log("texturePrefix: " + texturePrefix);
            sendMessage(finishFrame);

            // get all view children, loop until correct key

            GameObject obj = new GameObject(texturePrefix);
            obj.transform.parent = transform;
            views[visSyncId] = obj;

            for (int f = 0; f < viewsPerFrame; f++) {
                GameObject sharedTexture = new GameObject(texturePrefix+f);
                VisSyncSharedTexture tex = sharedTexture.AddComponent(typeof(VisSyncSharedTexture)) as VisSyncSharedTexture;
                tex.textureName = texturePrefix;
                sharedTexture.transform.parent = obj.transform;
            }

            /*foreach (Transform child in transform)
            {
                // check to see if it already exists
                Debug.Log(child.gameObject.name);
            }*/
        }
        else
        {
            sendMessage(finishFrame);
            queueSendInt(finishFrame,  0);
        }

    }
}
