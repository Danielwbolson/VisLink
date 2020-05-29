using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using System.Runtime.InteropServices;
using UnityEngine.UI;
using System;
using UnityEngine.Rendering;

public class VisSyncView : MonoBehaviour
{
    public String texturePrefix = "VisSync";



    /*[DllImport("VLUnityConnector")]
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

    public String texturePrefix = "VisSync";

    private bool initialized = false;
    private IntPtr startFrame;
    private IntPtr finishFrame;
    private IntPtr infoRequest;
    private IntPtr infoResponse;
    private Camera cam = null;
    private bool firstTime;

    void Update()
    {
        if (!initialized)
        {
            VisSyncClient client = gameObject.GetComponentInParent<VisSyncClient>();
            if (client.IsReady())
            {
                int api = client.GetAPI();
                startFrame = getMessageQueue(api, client.clientName + "-start");
                finishFrame = getMessageQueue(api, client.clientName + "-finish");

                initialized = true;
            }
        }

        if (initialized)
        {
            waitForMessage(startFrame);
            Debug.Log("recieved frame");
            int frameVal = queueRecieveInt(startFrame);
            Debug.Log("Id: " + frameVal);
            frameVal = queueRecieveInt(startFrame);
            Matrix4x4 proj = getMatrix4x4(startFrame);
            Matrix4x4 view = getMatrix4x4(startFrame);
            Matrix4x4 model = getMatrix4x4(startFrame);
            sendMessage(finishFrame);
            queueSendInt(finishFrame, 1);

            waitForMessage(startFrame);
            frameVal = queueRecieveInt(startFrame);
            string name = receiveString(startFrame);
            Debug.Log(frameVal);
            Debug.Log(name);
            sendMessage(finishFrame);

        }
    }*/

    // Update is called once per frame
    /*void Update2()
    {
        if (!initialized)
        {
            VisSyncClient client = gameObject.GetComponentInParent<VisSyncClient>();
            VisSyncSharedTexture sharedTexture = GetComponent<VisSyncSharedTexture>();
            if (client.IsReady() && sharedTexture.GetExternalTexture() != null)
            {
                int api = client.GetAPI();
                startFrame = getMessageQueue(api, sharedTexture.textureName + "-start");
                finishFrame = getMessageQueue(api, sharedTexture.textureName + "-finish");
                //finishFrame = getMessageQueue(api, sharedTexture.textureName + "-info-request");
                //finishFrame = getMessageQueue(api, sharedTexture.textureName + "-info-response");

                GameObject view = new GameObject("View");
                view.transform.parent = transform;
                cam = view.AddComponent(typeof(Camera)) as Camera;
                RenderTexture rt = new RenderTexture(sharedTexture.GetExternalTexture().width, sharedTexture.GetExternalTexture().height, 24, RenderTextureFormat.ARGB32);
                rt.Create();
                cam.targetTexture = rt;
                cam.clearFlags = CameraClearFlags.SolidColor;
                cam.backgroundColor = new Color(0, 0, 0, 0);

                CommandBuffer commandBuffer = new CommandBuffer();
                commandBuffer.CopyTexture(cam.targetTexture, sharedTexture.GetExternalTexture());
                cam.AddCommandBuffer(CameraEvent.AfterImageEffects, commandBuffer);

                initialized = true;
            }
        }
        
        if (initialized)
        {
            waitForMessage(startFrame);

            int frameVal = queueRecieveInt(startFrame);
            frameVal = queueRecieveInt(startFrame);
            Matrix4x4 proj = getMatrix4x4(startFrame);
            Matrix4x4 view = getMatrix4x4(startFrame);
            Matrix4x4 model = getMatrix4x4(startFrame);

            Matrix4x4 rh_to_lh = Matrix4x4.identity;
            rh_to_lh[0, 0] = -1.0f;

            cam.projectionMatrix = proj;
            cam.worldToCameraMatrix = view;
            cam.worldToCameraMatrix = rh_to_lh * cam.worldToCameraMatrix * model;

            sendMessage(finishFrame);
        }
    }*/
}
